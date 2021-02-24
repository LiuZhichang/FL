#include "iomanager.h"
#include "coroutine.h"
#include "logmanager.h"
#include "mutex.h"
#include "scheduler.h"
#include "macro.h"
#include "ScopeGuard.hpp"

#include <sys/epoll.h>
#include <fcntl.h>

namespace FL {

static FL::Logger::ptr syslog = FL_SYS_LOG();

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(Event event) {
    switch(event) {
    case IOManager::Event::READ:
        return read;
    case IOManager::Event::WRITE:
        return write;
    default:
        FL_ASSERT_2Arg(false, "getContext");
    }
    throw std::invalid_argument("getContext invalid event");
}

void IOManager::FdContext::resetContext(EventContext &ctx) {
    ctx.scheduler = nullptr;
    ctx.coroutine.reset();
    ctx.callback = nullptr;
}

void IOManager::FdContext::triggerEvent(IOManager::Event event) {
    FL_ASSERT(m_events & event);

    m_events = (Event)(m_events & ~event);
    EventContext& ctx = getContext(event);

    if (ctx.callback) {
        ctx.scheduler->schedule(&ctx.callback);
    } else {
        ctx.scheduler->schedule(&ctx.coroutine);
    }

    ctx.scheduler = nullptr;
    return;

}

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name)
    : Scheduler(threads, use_caller, name) {

    m_epfd = epoll_create(5000);
    FL_ASSERT(m_epfd > 0);

    int res = pipe(m_fds);
    FL_ASSERT(!res);

    // 注册epoll event
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_fds[0];

    // 设置非阻套接字
    int flag = fcntl(m_fds[0], F_GETFL, NULL);
    res = fcntl(m_fds[0], F_SETFL, flag | O_NONBLOCK);
    FL_ASSERT(!res);

    // 添加注册事件
    res = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_fds[0], &event);
    FL_ASSERT(!res);

    contextResize(32);
    // Scheduler 中的 , 默认启动开启
    start();
}

IOManager::~IOManager() {

    stop();
    close(m_epfd);
    close(m_fds[0]);
    close(m_fds[1]);

    for(size_t i = 0 ; i < m_fdContexts.size() ; ++i) {
        if(m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

void IOManager::contextResize(size_t size) {
    m_fdContexts.resize(size);

    for(size_t i = 0 ; i < m_fdContexts.size(); ++i) {
        if(!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {

    FdContext* ctx = nullptr;
    Mutex_t::ReadLock lock(m_mutex);
    if(m_fdContexts.size() > fd) {
        ctx = m_fdContexts[fd];
        lock.unlock();
    } else {
        lock.unlock();
        RWMutex::WriteLock lock2(m_mutex);
        contextResize(fd * 1.5);
        ctx = m_fdContexts[fd];
    }

    FdContext::Mutex_t::Lock lock3(ctx->mutex);

    if(FL_UNLICKLY(ctx->m_events & event)) {
        FL_LOG_ERROR(syslog) << "addevent assert fd=" << fd
                             << " event=" << event
                             << " fd_ctx.event=" << ctx->m_events;
        FL_ASSERT(!(ctx->m_events & event));
    }

    int op = ctx->m_events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event ep_event;
    ep_event.events = EPOLLIN | ctx->m_events | event;
    ep_event.data.ptr = ctx;

    int res = epoll_ctl(m_epfd, op, fd, &ep_event);
    if(res) {
        FL_LOG_ERROR(syslog) << "epoll_ctl (" << m_epfd << ","
                             << op << "," << fd << "," << ep_event.events << ");"
                             << res << " (" << errno << ") (" << strerror(errno) << ")"
                             << ctx->m_events;
        return -1;
    }

    ++ m_pending_event_count;
    ctx->m_events = (Event)(ctx->m_events | event);
    FdContext::EventContext& event_ctx = ctx->getContext(event);

    FL_ASSERT(!event_ctx.scheduler
              && !event_ctx.callback
              && !event_ctx.coroutine);

    event_ctx.scheduler = Scheduler::GetThis();
    if(cb) {
        event_ctx.callback.swap(cb);
    } else {
        event_ctx.coroutine = Coroutine::GetThis();
        FL_ASSERT(event_ctx.coroutine->getState() == Coroutine::State::EXEC);
    }
    return 0;
}

bool IOManager::delEvent(int fd, Event event) {
    Mutex_t::ReadLock lock(m_mutex);
    if(m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::Mutex_t::Lock lock2(fd_ctx->mutex);

    if(FL_UNLICKLY(!(fd_ctx->m_events & event))) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->m_events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event ep_event;
    // 水平触发
    ep_event.events = EPOLLET | new_events;
    ep_event.data.ptr = fd_ctx;

    int res = epoll_ctl(m_epfd, op, fd, &ep_event);
    if(res != 0) {
        FL_LOG_ERROR(syslog) << "epoll_ctl (" << m_epfd << ","
                             << op << "," << fd << "," << ep_event.events << ");"
                             << res << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    --m_pending_event_count;
    fd_ctx->m_events = new_events;
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}

bool IOManager::cancelEvent(int fd, Event event) {

    Mutex_t::ReadLock lock(m_mutex);
    if(m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::Mutex_t::Lock lock2(fd_ctx->mutex);
    if(FL_UNLICKLY(!(fd_ctx->m_events & event))) {
        return false;
    }

    Event new_events = (Event)(fd_ctx->m_events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event ep_event;
    ep_event.events = EPOLLET | new_events;
    ep_event.data.ptr = fd_ctx;

    int res = epoll_ctl(m_epfd, op, fd, &ep_event);
    if(res) {
        FL_LOG_ERROR(syslog) << "epoll_ctl (" << m_epfd << ","
                             << op << "," << fd << "," << ep_event.events << ");"
                             << res << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    fd_ctx->triggerEvent(event);
    --m_pending_event_count;
    return true;
}

bool IOManager::cancelAll(int fd) {
    Mutex_t::ReadLock lock(m_mutex);
    if(m_fdContexts.size() <= fd) {
        return false;
    }

    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::Mutex_t::Lock lock2(fd_ctx->mutex);
    if(!fd_ctx->m_events) {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int res = epoll_ctl(m_epfd, op, fd, &epevent);
    if(res != 0) {
        FL_LOG_ERROR(syslog) << "epoll_ctl(" << m_epfd << ", "
                             << op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << ");"
                             << res << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    if(fd_ctx->m_events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pending_event_count;
    }

    if(fd_ctx->m_events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pending_event_count;
    }

    FL_ASSERT(fd_ctx->m_events == 0);
    return true;
}

IOManager* IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle() {
    if(!Scheduler::hasIdleThreads()) {
        return;
    }

    int res = write(m_fds[1], "T", 1);
    FL_ASSERT(res == 1);
}

bool IOManager::stopping(uint64_t& timeout) {
    timeout = getNextTimer();
    return timeout == ~0ull
           && m_pending_event_count == 0
           && Scheduler::stopping();
}

bool IOManager::stopping() {
//    uint64_t timeout = 0;
//    return stopping(timeout);
    uint64_t timeout = 0;
    return stopping(timeout);
}

void IOManager::idle() {
    const uint64_t MAX_EVENTS = 256;
    epoll_event* events = new epoll_event[MAX_EVENTS]();
    ON_SCOPE_EXIT {
        delete[] events;
    };
//    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event * ptr) {
//        delete[] ptr;
//    });

    while(true) {
        uint64_t next_timeout = 0;
        if(FL_UNLICKLY(stopping(next_timeout))) {
            FL_LOG_INFO(syslog) << "name = " << getName()
                                << " idle stopping exit";
            break;
        }
        int res = 0;
        do {
            static const int MAX_TIMEOUT = 3000;
            if(next_timeout != ~0ull) {
                next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
            } else {
                next_timeout = MAX_TIMEOUT;
            }

            res = epoll_wait(m_epfd, events, MAX_EVENTS, (int)next_timeout);

            if(!(res < 0 && errno == EINTR)) {
                break;
            }
        } while(true);

        std::vector<std::function<void()>> cbs;
        listExpiredcb(cbs);
        if(!cbs.empty()) {
            schedule(cbs.begin(), cbs.end());
            cbs.clear();
        }

        for(int i = 0 ; i < res ; ++i) {
            epoll_event& event = events[i];
            if(event.data.fd == m_fds[0]) {
                uint8_t dummy[256];
                while(read(m_fds[0], dummy, sizeof(dummy)) > 0);
                continue;
            }

            FdContext* fd_ctx = (FdContext*)event.data.ptr;
            FdContext::Mutex_t::Lock lock(fd_ctx->mutex);
            if(event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->m_events;
            }
            int real_events = NONE;
            if(event.events & EPOLLIN) {
                real_events |= READ;
            }
            if(event.events & EPOLLOUT) {
                real_events |= WRITE;
            }

            if((fd_ctx->m_events & real_events) == NONE) {
                continue;
            }

            int left_events = (fd_ctx->m_events & ~real_events);
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;

            int res2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(res2) {
                FL_LOG_ERROR(syslog) << "epoll_ctl(" << m_epfd << ", "
                                     << op << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << ");"
                                     << res << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }
            if(real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --m_pending_event_count;
            }
            if(real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_pending_event_count;
            }

        }
        Coroutine::ptr cur = Coroutine::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->swapOut();
    }

}

void IOManager::onTimerInsertedAtFront() {
    tickle();
}

}
