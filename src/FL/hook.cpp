#include "hook.h"
#include "config.h"
#include "coroutine.h"
#include "fd_manager.h"
#include "iomanager.h"
#include "logmanager.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

namespace FL {

static ConfigVar<int>::ptr g_tcp_connect_timeout =
    Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX)\
	XX(sleep)		\
	XX(usleep)		\
	XX(nanosleep)	\
	XX(socket)		\
	XX(connect)		\
	XX(accept)		\
	XX(read)		\
	XX(readv)		\
	XX(recv)		\
	XX(recvfrom)	\
	XX(recvmsg)		\
	XX(write)		\
	XX(writev)		\
	XX(send)		\
	XX(sendto)		\
	XX(sendmsg)		\
	XX(close)		\
	XX(fcntl)		\
	XX(ioctl)		\
	XX(setsockopt)	\
	XX(getsockopt)


void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
    is_inited = true;
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT,#name);
    HOOK_FUN(XX)
#undef XX
}

bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

static uint64_t s_connect_timeout = -1;
struct __Hook_Initer {
    __Hook_Initer() {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getVal();

        g_tcp_connect_timeout->addListener([](const int& old_val, const int& new_val) {
            FL_LOG_INFO(FL_LOG_ROOT()) << "tcp connect timeout changed from " << old_val << " to " << new_val;
            s_connect_timeout = new_val;
        });
    }
};
static __Hook_Initer s_hook_initer;

struct timer_info {
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name
                     , uint32_t event, int timeout_so, Args&&... args) {
    if(!t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }

    FL::FdCtx::ptr ctx = FL::fd_manager::GetInstance()->get(fd);
    if(!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    while(n == -1 && errno == EAGAIN) {
        n = fun(fd, std::forward<Args>(args)...);
    }
    if(n == -1 && errno == EAGAIN) {
        IOManager* iom = IOManager::GetThis();
        Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if(to != (uint64_t) -1) {
            timer = iom->addConditionTimer(to, [winfo, fd, iom, event]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return ;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (IOManager::Event)(event));
            }, winfo);
        }

        int res = iom->addEvent(fd, (IOManager::Event)(event));
        if(res) {
            FL_LOG_ERROR(FL_SYS_LOG()) << hook_fun_name << " addEvent("
                                       << fd << ", " << event << ")";
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else {
            Coroutine::YieldToSuspend();
            if(timer) {
                timer->cancel();
            }
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    return n;
}

extern "C" {

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

    unsigned int sleep(unsigned int seconds) {
        if(!t_hook_enable) {
            return sleep_f(seconds);
        }

        FL::Coroutine::ptr cor = FL::Coroutine::GetThis();
        FL::IOManager* iom = FL::IOManager::GetThis();
        iom->addTimer(seconds * 1000, std::bind((void(FL::Scheduler::*)
                                                (FL::Coroutine::ptr, int thread))&FL::IOManager::schedule
                                                , iom, cor, -1));
        FL::Coroutine::YieldToSuspend();
        return 0;
    }

    int usleep(useconds_t usec) {
        if(!t_hook_enable) {
            return sleep_f(usec);
        }

        FL::Coroutine::ptr cor = FL::Coroutine::GetThis();
        FL::IOManager* iom = FL::IOManager::GetThis();

        iom->addTimer(usec / 1000, std::bind((void(FL::Scheduler::*)
                                              (FL::Coroutine::ptr, int thread))&FL::IOManager::schedule
                                             , iom, cor, -1));
        FL::Coroutine::YieldToSuspend();
        return 0;
    }

    int nanosleep(const struct timespec* req, struct timespec* rem) {
        if(!t_hook_enable) {
            return nanosleep_f(req, rem);
        }

        int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
        FL::Coroutine::ptr cor = FL::Coroutine::GetThis();
        FL::IOManager* iom = FL::IOManager::GetThis();
        iom->addTimer(timeout_ms, std::bind((void(FL::Scheduler::*)
                                             (FL::Coroutine::ptr, int thread))&FL::IOManager::schedule
                                            , iom, cor, -1));
        FL::Coroutine::YieldToSuspend();
        return 0;
    }

    int socket(int __domain, int __type, int __protocol) {
        if(!t_hook_enable) {
            return socket_f(__domain, __type, __protocol);
        }
        int fd = socket_f(__domain, __type, __protocol);
        if(fd == -1) {
            return fd;
        }
        FL::fd_manager::GetInstance()->get(fd, true);
        return fd;
    }

    int connect_timeout(int __fd, const struct sockaddr *__addr, socklen_t __len, uint64_t timeout) {
        if(!t_hook_enable) {
            return connect_f(__fd, __addr, __len);
        }
        FdCtx::ptr ctx = fd_manager::GetInstance()->get(__fd);
        if(!ctx || ctx->isClose()) {
            errno = EBADF;
            return -1;
        }

        if(!ctx->isSocket()) {
            return connect_f(__fd, __addr, __len);
        }

        if(ctx->getUserNonblock()) {
            return connect_f(__fd, __addr, __len);
        }

        int n = connect_f(__fd, __addr, __len);
        if(n == 0) {
            return 0;
        } else if(n != -1 || errno != EINPROGRESS) {
            return n;
        }

        IOManager* iom = IOManager::GetThis();
        Timer::ptr timer;
        std::shared_ptr<timer_info> tinfo(new timer_info);
        std::weak_ptr<timer_info> winfo(tinfo);
        if(timeout != (uint64_t) -1) {
            timer = iom->addConditionTimer(timeout, [winfo, __fd, iom]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(__fd, IOManager::WRITE);
            }, winfo);
        }
        int res = iom->addEvent(__fd, IOManager::WRITE);
        if(res == 0) {
            Coroutine::YieldToSuspend();
            if(timer) {
                timer->cancel();
            }
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
        } else {
            if(timer) {
                timer->cancel();
            }
            FL_LOG_ERROR(FL_SYS_LOG()) << "coonect addEvent(" << __fd << ", WRITE) error";
        }
        int error = 0;
        socklen_t len = sizeof(int);
        if(-1 == getsockopt(__fd, SOL_SOCKET, SO_ERROR, &errno, &len)) {
            return -1;
        }
        if(!error) {
            return 0;
        } else {
            errno = error;
            return -1;
        }
    }

    int connect(int __fd, const struct sockaddr *__addr, socklen_t __len) {
        return connect_timeout(__fd, __addr, __len, s_connect_timeout);
    }

    int accept(int __fd, struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len) {
        int fd = do_io(__fd, accept_f, "accept", IOManager::READ, SO_RCVTIMEO, __addr, __addr_len);
        if(fd >= 0) {
            FL::fd_manager::GetInstance()->get(fd, true);
        }
        return fd;
    }

    ssize_t read(int __fd, void *__buf, size_t __nbytes) {
        return do_io(__fd, read_f, "read", IOManager::READ, SO_RCVTIMEO, __buf, __nbytes);
    }

    ssize_t readv(int __fd, const struct iovec *__iovec, int __count) {
        return do_io(__fd, readv_f, "readv", IOManager::READ, SO_RCVTIMEO, __iovec, __count);
    }

    ssize_t recv(int __fd, void *__buf, size_t __n, int __flags) {
        return do_io(__fd, recv_f, "recv", IOManager::READ, SO_RCVTIMEO, __buf, __n, __flags);
    }

    ssize_t recvfrom(int __fd, void *__restrict __buf, size_t __n, int __flags, struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len) {
        return do_io(__fd, recvfrom_f, "recvfrom", IOManager::READ, SO_RCVTIMEO, __buf, __n, __flags, __addr, __addr_len);
    }

    ssize_t recvmsg(int __fd, struct msghdr *__message, int __flags) {
        return do_io(__fd, recvmsg_f, "recvmsg", IOManager::READ, SO_RCVTIMEO, __message, __flags);
    }

    ssize_t write(int __fd, const void *__buf, size_t __n) {
        return do_io(__fd, write_f, "write", IOManager::WRITE, SO_SNDTIMEO, __buf, __n);
    }

    ssize_t writev(int __fd, const struct iovec *__iovec, int __count) {
        return do_io(__fd, writev_f, "writev", IOManager::WRITE, SO_SNDTIMEO, __iovec, __count);
    }

    ssize_t send(int __fd, const void *__buf, size_t __n, int __flags) {
        return do_io(__fd, send_f, "send", IOManager::WRITE, SO_SNDTIMEO, __buf, __n, __flags);
    }

    ssize_t sendto(int __fd, const void *__buf, size_t __n, int __flags, const struct sockaddr *__addr, socklen_t __addr_len) {
        return do_io(__fd, sendto_f, "sendto", IOManager::WRITE, SO_SNDTIMEO, __buf, __n, __flags, __addr, __addr_len);
    }

    ssize_t sendmsg(int __fd, const struct msghdr *__message, int __flags) {
        return do_io(__fd, sendmsg_f, "sendmsg", IOManager::WRITE, SO_SNDTIMEO, __message, __flags);
    }

    int close(int fd) {
        if(!t_hook_enable) {
            return close_f(fd);
        }

        FdCtx::ptr ctx = fd_manager::GetInstance()->get(fd);
        if(ctx) {
            auto iom = IOManager::GetThis();
            if(iom) {
                iom->cancelAll(fd);
            }
            fd_manager::GetInstance()->del(fd);
        }
        return close_f(fd);
    }

    int fcntl(int __fd, int __cmd, ...) {
        va_list va;
        va_start(va, __cmd);
        switch(__cmd) {
        case F_SETFL: {
            int arg = va_arg(va, int);
            va_end(va);
            FL::FdCtx::ptr ctx = FL::fd_manager::GetInstance()->get(__fd);
            if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                return fcntl_f(__fd, __cmd, arg);
            }
            ctx->setUserNonblock(arg & O_NONBLOCK);
            if(ctx->getSysNonblock()) {
                arg |= O_NONBLOCK;
            } else {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(__fd, __cmd, arg);
        }
        break;
        case F_GETFL: {
            va_end(va);
            int arg = fcntl_f(__fd, __cmd);
            FL::FdCtx::ptr ctx = FL::fd_manager::GetInstance()->get(__fd);
            if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                return arg;
            }
            if(ctx->getUserNonblock()) {
                return arg | O_NONBLOCK;
            } else {
                return arg & ~O_NONBLOCK;
            }
        }
        break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(__fd, __cmd, arg);
        }
        break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
        {
            va_end(va);
            return fcntl_f(__fd, __cmd);
        }
        break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK: {
            struct flock* arg = va_arg(va, struct flock*);
            va_end(va);
            return fcntl_f(__fd, __cmd, arg);
        }
        break;
        case F_GETOWN_EX:
        case F_SETOWN_EX: {
            struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
            va_end(va);
            return fcntl_f(__fd, __cmd, arg);
        }
        break;
        default:
            va_end(va);
            return fcntl_f(__fd, __cmd);
        }

    }

    int ioctl(int __fd, unsigned long __request, ...) {
        va_list va;
        va_start(va, __request);
        void* arg = va_arg(va, void*);
        va_end(va);

        if(FIONBIO == __request) {
            bool user_nonblock = !!*(int*)arg;
            FL::FdCtx::ptr ctx = FL::fd_manager::GetInstance()->get(__fd);
            if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                return ioctl_f(__fd, __request, arg);
            }
            ctx->setUserNonblock(user_nonblock);
        }
        return ioctl_f(__fd, __request, arg);
    }

    int getsockopt(int __fd, int __level, int __optname, void *__optval, socklen_t *__optlen) {
        return getsockopt_f(__fd, __level, __optname, __optval, __optlen);
    }

    int setsockopt(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen) {
        if(!FL::t_hook_enable) {
            return setsockopt_f(__fd, __level, __optname, __optval, __optlen);
        }
        if(__level == SOL_SOCKET) {
            if(__optname == SO_RCVTIMEO || __optname == SO_SNDTIMEO) {
                FL::FdCtx::ptr ctx = FL::fd_manager::GetInstance()->get(__fd);
                if(ctx) {
                    const timeval* v = (const timeval*)__optval;
                    ctx->setTimeout(__optname, v->tv_sec * 1000 + v->tv_usec / 1000);
                }
            }
        }
        return setsockopt_f(__fd, __level, __optname, __optval, __optlen);
    }

}
}
