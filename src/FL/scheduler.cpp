#include "scheduler.h"
#include "coroutine.h"
#include "logmanager.h"
#include "hook.h"
#include "macro.h"
#include "util.h"
#include <memory>
#include <string>

namespace FL {

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local	Coroutine* t_scheduler_coroutine = nullptr;

Scheduler::Scheduler(size_t tcount, bool use_caller, const std::string& name)
    : m_name(name) {

    // 线程数量必须大于0
    FL_ASSERT(tcount > 0);

    // if(tcount <= 0)
    //     tcount = 1;

    // 是否使用当前使用的线程
    if(use_caller) {

        Coroutine::GetThis();
        --tcount;

        FL_ASSERT(GetThis() == nullptr);
        t_scheduler = this;
        m_mainCoroutine.reset(new Coroutine(std::bind(&Scheduler::run, this), 0, true));
        Thread::SetName(m_name);

        t_scheduler_coroutine = m_mainCoroutine.get();
        m_mainThread = UT::GetThreadId();
        m_threadIds.push_back(m_mainThread);
    } else {
        m_mainThread = -1;
    }
    m_threadCount = tcount;
}

Scheduler::~Scheduler() {
    FL_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Coroutine* Scheduler::GetMainCoroutine() {
    return t_scheduler_coroutine;
}

void Scheduler::start() {

    Mutex_t::Lock lock(m_mutex);
    if(!m_stopping) {
        return ;
    }
    m_stopping = false;
    FL_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for(size_t i = 0 ; i < m_threadCount ; ++i) {
        m_threads[i].reset(new Thread(m_name + " " + std::to_string(i), std::bind(&Scheduler::run, this)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
}

void Scheduler::stop() {
    m_autoStop = true;
    if(m_mainCoroutine
            && m_threadCount == 0
            && (m_mainCoroutine->getState() == Coroutine::State::TERMINATE
                || m_mainCoroutine->getState() == Coroutine::State::INIT)) {
        FL_LOG_INFO(FL_LOG_ROOT()) << this << " stopped";
        m_stopping = true;

        if(stopping()) {
            return;
        }
    }

    if(m_mainThread != -1) {
        FL_ASSERT(GetThis() == this);
    } else {
        FL_ASSERT(GetThis() != this);
    }

    m_stopping = true;

    for(size_t i = 0 ; i < m_threadCount; ++i) {
        tickle();
    }

    if(m_mainCoroutine) {
        tickle();
    }
    if(m_mainCoroutine) {
        if(!stopping()) {
            m_mainCoroutine->call();
        }
    }

    std::vector<Thread::ptr> threads;
    {
        Mutex_t::Lock lock(m_mutex);
        threads.swap(m_threads);
    }

    for(auto thread : threads) {
        thread->join();
    }

}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    FL_LOG_DEBUG(FL_LOG_ROOT()) << "< Scheduler: " << m_name << " Run >";

    set_hook_enable(true);
    setThis();
    if(UT::GetThreadId() != m_mainThread) {
        t_scheduler_coroutine = Coroutine::GetThis().get();
    }

    Coroutine::ptr idle_Coroutine(new Coroutine(std::bind(&Scheduler::idle, this)));
    Coroutine::ptr cb_Coroutine;

    SchedulerDetails sd;
    while(true) {
        sd.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            Mutex_t::Lock lock(m_mutex);
            auto it = m_coroutines.begin();
            while(it != m_coroutines.end()) {
                if(it->thread_id != -1 && it->thread_id != UT::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                FL_ASSERT(it->coroutine || it->callback);
                if(it->coroutine && it->coroutine->getState() == Coroutine::State::EXEC) {
                    ++it;
                    continue;
                }

                sd = *it;
                m_coroutines.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_coroutines.end();
        }

        if(tickle_me) {
            tickle();
        }

        if(sd.coroutine && (sd.coroutine->getState() != Coroutine::State::TERMINATE
                            && sd.coroutine->getState() != Coroutine::State::EXCEPT)) {
            sd.coroutine->swapIn();
            --m_activeThreadCount;

            if(sd.coroutine->getState() == Coroutine::State::READY) {
                schedule(sd.coroutine);
            } else if(sd.coroutine->getState() != Coroutine::State::TERMINATE
                      && sd.coroutine->getState() != Coroutine::State::EXCEPT) {
                sd.coroutine->m_state = Coroutine::State::SUSPEND;
            }
            sd.reset();
        } else if(sd.callback) {
            if(cb_Coroutine) {
                cb_Coroutine->reset(sd.callback);
            } else {
                cb_Coroutine.reset(new Coroutine(sd.callback));
            }
            sd.reset();
            cb_Coroutine->swapIn();
            --m_activeThreadCount;
            if(cb_Coroutine->getState() == Coroutine::State::READY) {
                schedule(cb_Coroutine);
                cb_Coroutine.reset();
            } else if(cb_Coroutine->getState() == Coroutine::State::EXCEPT
                      || cb_Coroutine->getState() == Coroutine::State::TERMINATE) {
                cb_Coroutine->reset(nullptr);
            } else {
                cb_Coroutine->m_state = Coroutine::State::SUSPEND;
                cb_Coroutine.reset();
            }
        } else {
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            if(idle_Coroutine->getState() == Coroutine::State::TERMINATE) {
                FL_LOG_INFO(FL_LOG_ROOT()) << "< Idle Coroutine state[TERMINATE] >";
                break;
            }

            ++m_idleThreadCount;
            idle_Coroutine->swapIn();
            --m_idleThreadCount;
            if(idle_Coroutine->getState() != Coroutine::State::TERMINATE
                    && idle_Coroutine->getState() != Coroutine::State::EXCEPT) {
                idle_Coroutine->m_state = Coroutine::State::SUSPEND;
            }
        }
    }
}

void Scheduler::tickle() {
    FL_LOG_INFO(FL_LOG_ROOT()) << "< Tickle >";
}

bool Scheduler::stopping() {
    Mutex_t::Lock lock(m_mutex);
    return m_autoStop && m_stopping
           && m_coroutines.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    FL_LOG_INFO(FL_LOG_ROOT()) << "< Idle >";
    while(!stopping()) {
        Coroutine::YieldToSuspend();
    }
}

}
