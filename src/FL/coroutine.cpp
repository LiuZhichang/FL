#include "config.h"
#include "coroutine.h"
#include "logmanager.h"
#include "macro.h"
#include "scheduler.h"

namespace FL {

static Logger::ptr syslog = FL_SYS_LOG();

static std::atomic<uint64_t> s_Coroutine_id{0};		// 协程id
static std::atomic<uint64_t> s_Coroutine_count{0};	// 协程数量

static thread_local Coroutine* t_Coroutine = nullptr;
static thread_local Coroutine::ptr t_threadCoroutine = nullptr;
static thread_local std::atomic<uint64_t> s_cur_cnt{0};

static ConfigVar<uint32_t>::ptr g_Coroutine_stack_size =
    Config::Lookup<uint32_t>("coroutine.stack_size", (1 << 17), "Coroutine stack size");

class StackAllocator {
  public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }
    static void Dealloc(void* val_p, size_t size) {
        return free(val_p);
    }
  private:
};

//using allocate_s = StackAllocator;
typedef StackAllocator allocator_s;

Coroutine::Coroutine() {
    m_state = State::EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        FL_ASSERT_2Arg(false, "getcontext");
    }
    ++ s_Coroutine_count;
    ++ s_cur_cnt;
    FL_LOG_DEBUG(syslog) << "Coroutine: Coroutine main";
}

Coroutine::Coroutine(std::function<void()> callback, size_t stacksize, bool usr_caller)
    : m_id(++s_Coroutine_id)
    , m_cb(callback) {
    ++s_Coroutine_count;
    ++ s_cur_cnt;

    m_stacksize = stacksize ? stacksize : g_Coroutine_stack_size->getVal();

    m_stack = allocator_s::Alloc(m_stacksize);
    if(getcontext(&m_ctx)) {
        FL_ASSERT_2Arg(false, "getcontext");
    }

//    if(t_Coroutine) {
//        m_ctx.uc_link = &t_Coroutine->m_ctx;
//    } else {
//        m_ctx.uc_link = nullptr;
//    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if(!usr_caller)
        makecontext(&m_ctx, &Coroutine::MainFunc, 0);
    else
        makecontext(&m_ctx, &Coroutine::CallerMainFunc, 0);
    FL_LOG_DEBUG(syslog) << "Coroutine::Coroutine id=" << m_id;
}

Coroutine::~Coroutine() {
    --s_Coroutine_count;
    --s_cur_cnt;
    if(m_stack) {
        FL_ASSERT(m_state == State::TERMINATE
                  || m_state == State::INIT
                  || m_state == State::EXCEPT);
        allocator_s::Dealloc(m_stack, m_stacksize);
    } else {
        FL_ASSERT(!m_cb);
        FL_ASSERT(m_state == State::EXEC);

        Coroutine* cur = t_Coroutine;
        if(cur == this)
            SetThis(nullptr);
    }
    FL_LOG_DEBUG(syslog) << "Coroutine::~Coroutine id=" << m_id
                                << " total=" << s_Coroutine_count
                                << " [current thread coroutine's cnt=" << s_cur_cnt << "]";
}

void Coroutine::reset(std::function<void()> callback) {
    FL_ASSERT(m_stack);
    FL_ASSERT(m_state == State::TERMINATE
              || m_state == State::EXCEPT
              || m_state == State::INIT);
    m_cb = callback;

    if(getcontext(&m_ctx)) {
        FL_ASSERT_2Arg(&m_ctx, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Coroutine::MainFunc, 0);
    m_state = State::INIT;
}

void Coroutine::swapIn() {
    SetThis(this);
    FL_ASSERT(m_state != State::EXEC);

    m_state = State::EXEC;
//    if(swapcontext(&t_threadCoroutine->m_ctx, &m_ctx))
    if(swapcontext(&Scheduler::GetMainCoroutine()->m_ctx, &m_ctx))
        FL_ASSERT_2Arg(false, "swapcontext");
}

void Coroutine::swapOut() {
    SetThis(t_threadCoroutine.get());

//    if(swapcontext(&m_ctx,&t_threadCoroutine->m_ctx))
    if(swapcontext(&m_ctx, &Scheduler::GetMainCoroutine()->m_ctx))
        FL_ASSERT_2Arg(false, "swapcontext");
}

void Coroutine::call() {
    SetThis(this);
    m_state = State::EXEC;
    if(swapcontext(&t_threadCoroutine->m_ctx, &m_ctx))
        FL_ASSERT_2Arg(false, "swapcontext");
}

void Coroutine::back() {
    SetThis(t_threadCoroutine.get());

    if(swapcontext(&m_ctx, &t_threadCoroutine->m_ctx)) {
        FL_ASSERT_2Arg(false, "swapcontext");
    }
}

void Coroutine::SetThis(Coroutine* Coroutine) {
    t_Coroutine = Coroutine;
}

Coroutine::ptr Coroutine::GetThis() {
    if(t_Coroutine)
        return t_Coroutine->shared_from_this();
    Coroutine::ptr main_Coroutine(new Coroutine);
    FL_ASSERT(t_Coroutine == main_Coroutine.get());
    t_threadCoroutine = main_Coroutine;
    return t_Coroutine->shared_from_this();
}

void Coroutine::YieldToReady() {
    Coroutine::ptr cur = GetThis();
    FL_ASSERT(cur->m_state == State::EXEC);
    cur->m_state = State::READY;
    cur->swapOut();
}

void Coroutine::YieldToSuspend() {
    Coroutine::ptr cur = GetThis();
    FL_ASSERT(cur->m_state == State::EXEC);
//    cur->m_state = State::SUSPEND;

    cur->swapOut();
}

uint64_t Coroutine::TotalCoroutines() {
    return s_Coroutine_count;
}

uint64_t Coroutine::CurThreadCorCnt() {
    return s_cur_cnt;
}

void Coroutine::MainFunc() {
    Coroutine::ptr cur = GetThis();
    FL_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = State::TERMINATE;
    } catch(std::exception& e) {
        cur->m_state = State::EXCEPT;
        FL_LOG_ERROR(syslog) << "Coroutine Except: " << e.what()
                                   << " coroutine_id=" << cur->getId()
                                   << std::endl
                                   << FL::UT::BacktraceToString();
    } catch(...) {
        cur->m_state = State::EXCEPT;
        FL_LOG_ERROR(syslog) << "Coroutine Except "
                                   << " Coroutine_id=" << cur->getId()
                                   << std::endl
                                   << FL::UT::BacktraceToString();
    }


    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    FL_ASSERT_2Arg(false, "never reach coroutine_id=" + std::to_string(raw_ptr->getId()));
}

void Coroutine::CallerMainFunc() {
    Coroutine::ptr cur = GetThis();
    FL_ASSERT(cur);

    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = State::TERMINATE;
    } catch (std::exception& e) {
        cur->m_state = State::EXEC;
        FL_LOG_ERROR(syslog) << "Coroutine Except: " << e.what()
                                   << " coroutine_id=" << cur->getId()
                                   << std::endl
                                   << FL::UT::BacktraceToString(10);
    } catch(...) {
        cur->m_state = State::EXEC;
        FL_LOG_ERROR(syslog) << "Coroutine Except: "
                                   << " coroutine_id=" << cur->getId()
                                   << std::endl
                                   << FL::UT::BacktraceToString(10);
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    FL_ASSERT_2Arg(false, "never reach coroutine_id=" + std::to_string(raw_ptr->getId()));
}

uint64_t Coroutine::GetCoroutineId() {
    if(t_Coroutine)
        return t_Coroutine->getId();
    return 0;
}

}
