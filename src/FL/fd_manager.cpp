#include "fd_manager.h"
#include "hook.h"

namespace FL {

FdCtx::FdCtx(int fd)
    : m_isInit(false)
    , m_isSocket(false)
    , m_sysNonblock(false)
    , m_userNonblock(false)
    , m_isClosed(false)
    , m_fd(fd)
    , m_recvTimeout(-1)
    , m_sendTimeout(-1) {
    init();
}

FdCtx::~FdCtx() {

}

bool FdCtx::init() {
    if(m_isInit) {
        return true;
    }
    m_recvTimeout = -1;
    m_sendTimeout = -1;

    struct stat fd_stat;
    if(fstat(m_fd, &fd_stat) == -1) {
        m_isInit = false;
        m_isSocket = false;
    } else {
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    if(m_isSocket) {
        int flags = fcntl_f(m_fd, F_GETFL, 0);
        if(!(flags & O_NONBLOCK)) {
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_sysNonblock = true;
    } else {
        m_sysNonblock = false;
    }

    m_userNonblock = false;
    m_isClosed = false;
    return m_isInit;
}

void FdCtx::setTimeout(int type, uint64_t time) {
    if(type == SO_RCVTIMEO) {
        m_recvTimeout = time;
    } else if(type == SO_SNDTIMEO) {
        m_sendTimeout = time;
    }
}

uint64_t FdCtx::getTimeout(int type) {
    if(type == SO_RCVTIMEO) {
        return m_recvTimeout;
    } else if(type == SO_SNDTIMEO) {
        return m_sendTimeout;
    }
    return -1;
}

FdManager::FdManager() {
    m_datas.resize(64);
}

FdCtx::ptr FdManager::get(int fd, bool auto_create) {
    Mutex_t::ReadLock lock(m_mutex);
    if(m_datas.size() <= fd) {
        if(auto_create == false) {
            return nullptr;
        }
    } else {
        if(m_datas[fd] || !auto_create) {
            return m_datas[fd];
        }
    }
    lock.unlock();

    Mutex_t::WriteLock lock2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    m_datas[fd] = ctx;
    return ctx;
}

void FdManager::del(int fd) {
	Mutex_t::WriteLock lock(m_mutex);
	if(m_datas.size() <= fd) {
		return;
	}
	m_datas[fd].reset();
}

}
