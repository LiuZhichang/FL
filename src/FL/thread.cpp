#include "thread.h"

namespace FL {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "unknow";


Thread::Thread( const std::string&name, std::function<void()> cb)
    : m_name(name)
    , m_callback(cb) {
    if(name.empty()) {
        m_name = "unknow";
    }

    int res = pthread_create(&m_thread, nullptr, &Thread::run, this);

    if(res) {
        FL_LOG_ERROR(FL_SYS_LOG()) << "pthread_create thread failed, result=" << res
                                   << " name=" << m_name << " " << strerror(res);
        throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();
}

Thread::~Thread() {
    if(m_thread)
        pthread_detach(m_thread);
}

void Thread::join() {
    if(m_thread) {
        int res = pthread_join(m_thread, nullptr);
        if(res) {
            FL_LOG_ERROR(FL_SYS_LOG()) << "pthread_create thread failed, result=" << res
                                       << " name=" << m_name << " " << strerror(res);
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

void* Thread::run(void* arg) {
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = FL::UT::GetThreadId();

    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    std::function<void()> callback;

    // 防止有智能指针时释放不掉
    callback.swap(thread->m_callback);

    thread->m_semaphore.notify();
    callback();

    return 0;
}

Thread* Thread::GetThis() {
    return t_thread;
}

const std::string& Thread::GetName() {
    return t_thread_name;
}

void Thread::SetName(const std::string& name) {
    if(name.empty())
        return;
    if(t_thread)
        t_thread->m_name = name;
    t_thread_name = name;
}

}
