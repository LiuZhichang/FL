#include "socket.h"
#include "address.h"
#include "fd_manager.h"
#include "logmanager.h"
#include "iomanager.h"
#include "endian.h"
#include "hook.h"
#include "macro.h"

#include <cstring>
#include <netinet/tcp.h>
#include <sstream>
#include <sys/socket.h>

namespace FL {

static FL::Logger::ptr syslog = FL_SYS_LOG();

Socket::ptr Socket::CreateTCP(Address::ptr address) {
    Socket::ptr sock(new Socket(address->getFamily(), Type::TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDP(Address::ptr address) {
    Socket::ptr sock(new Socket(address->getFamily(), Type::UDP, 0));
    return sock;
}

Socket::ptr Socket::CreateTCPSocket() {
    Socket::ptr sock(new Socket(IPv4, Type::TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPSocket() {
    Socket::ptr sock(new Socket(IPv4, Type::UDP, 0));
    return sock;
}

Socket::ptr Socket::CreateTCPSocket6() {
    Socket::ptr sock(new Socket(IPv6, Type::TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPSocket6() {
    Socket::ptr sock(new Socket(IPv6, Type::UDP, 0));
    return sock;
}
Socket::ptr Socket::CreateUnixTCPSocket() {
    Socket::ptr sock(new Socket(UNIX, TCP, 0));
    return sock;
}
Socket::ptr Socket::CreateUnixUDPSocket() {
    Socket::ptr sock(new Socket(UNIX, UDP, 0));
    return sock;
}

Socket::Socket(int family, int type, int protocol)
    : m_sock(-1)
    , m_family(family)
    , m_type(type)
    , m_protocol(protocol)
    , m_isConnected(false) {

}

Socket::~Socket() {
    close();
}

int64_t Socket::getSendTimeout() {
    FdCtx::ptr ctx = fd_manager::GetInstance()->get(m_sock);
    if(ctx)  {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setSendTimeout(uint64_t time) {
    struct timeval tv {
        int(time / 1000), int(time % 1000 * 1000)
    };
    setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

int64_t Socket::getRecvTimeout() {
    FdCtx::ptr ctx = fd_manager::GetInstance()->get(m_sock);
    if(ctx)  {
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeout(uint64_t time) {
    struct timeval tv {
        int(time / 1000), int(time % 1000 * 1000)
    };
    setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}

bool Socket::getOption(int level, int opt, void* result, socklen_t* len) {
    int rt = getsockopt(m_sock, level, opt, result, (socklen_t*)len);
    if(rt) {
        FL_LOG_DEBUG(FL_LOG_ROOT()) << "getOption socket = " << m_sock
                                    << " level = " << level << " option = " << opt
                                    << " errno = " << errno << " error info = " << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level, int opt, const void* result, socklen_t len) {
    int rt = setsockopt(m_sock, level, opt, result, (socklen_t)len);
    if(rt) {
        FL_LOG_DEBUG(FL_LOG_ROOT()) << "getOption socket = " << m_sock
                                    << " level = " << level << " option = " << opt
                                    << " errno = " << errno << " error info = " << strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept() {
    Socket::ptr sock(new Socket(m_family, m_type, m_protocol));
    int newsock = ::accept(m_sock, nullptr, nullptr);
    if(newsock == -1) {
        FL_LOG_ERROR(syslog) << "accept(" << m_sock << ") errno = "
                             << errno << " error info = " << strerror(errno);
        return nullptr;
    }
    if(sock->init(newsock)) {
        return sock;
    }
    return nullptr;
}

bool Socket::bind(const Address::ptr addr) {
    if(!isValid()) {
        newSock();
        if(FL_UNLICKLY(!isValid())) {
            return false;
        }
    }
    if(FL_UNLICKLY(addr->getFamily() != m_family)) {
        FL_LOG_ERROR(syslog) << "bind sock.family("
                             << m_family << ") addr.family(" << addr->getFamily()
                             << ") not equal, addr = " << addr->toString();
        return false;
    }

    if(::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
        FL_LOG_ERROR(syslog) << "bind error errno = " << errno
                             << " error info = " << strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {
    m_remoteAddress = addr;
    if(!isValid()) {
        newSock();
        if(FL_UNLICKLY(!isValid())) {
            return false;
        }
    }
    if(FL_UNLICKLY(addr->getFamily() != m_family)) {
        FL_LOG_ERROR(syslog) << "connect sock.family("
                             << m_family << ") addr.family(" << addr->getFamily()
                             << ") not equal, addr = " << addr->toString();
        return false;
    }

    if(timeout_ms == (uint64_t) -1) {
//      int rt = ::connect(m_sock, addr->getAddr(), addr->getAddrLen());
//		FL_LOG_DEBUG(FL_LOG_ROOT()) << rt;
        if(::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
//      if(rt) {
            FL_LOG_ERROR(syslog) << "scok = " << m_sock << " connect("
                                 << addr->toString()
                                 << ") error errno = " << errno << " errstr = " << strerror(errno);
            close();
            return false;
        }
    } else {
        if(::connect_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
            FL_LOG_ERROR(syslog) << "scok = " << m_sock << " connect("
                                 << addr->toString()
                                 << ") timeout = " << timeout_ms
                                 << "error errno = " << errno << " errstr = " << strerror(errno);
            close();
            return false;
        }
    }
    m_isConnected = true;
    getRemoteAddress();
    getLocalAddress();
    return true;
}

bool Socket::listen(int backlog) {
    if(!isValid()) {
        FL_LOG_ERROR(syslog) << "listen error sock = -1";
        return false;
    }
    if(::listen(m_sock, backlog)) {
        FL_LOG_ERROR(syslog) << "listen error errno = " << errno
                             << " errstr = " << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::close() {
    if(!m_isConnected && m_sock == -1) {
        return true;
    }

    m_isConnected = false;
    if(m_sock != -1) {
        ::close(m_sock);
        m_sock = -1;
    }
    return false;
}

int Socket::send(const void* buffer, size_t len, int flags) {
    if(isConnected()) {
        return ::send(m_sock, buffer, len, flags);
    }
    return -1;
}

int Socket::send(const iovec* buffers, size_t len, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = len;
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::sendTo(const void* buffer, size_t len, const Address::ptr to, int flags) {
    if(isConnected()) {
        return ::sendto(m_sock, buffer, len, flags, to->getAddr(), to->getAddrLen());
    }
    return -1;
}

int Socket::sendTo(const iovec* buffers, size_t len, const Address::ptr to, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = len;
        msg.msg_name = (void*)to->getAddr();
        msg.msg_namelen = to->getAddrLen();
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recv(void* buffer, size_t len, int flags) {
    if(isConnected()) {
        return ::recv(m_sock, buffer, len, flags);
    }
    return -1;
}

int Socket::recv(iovec* buffers, size_t len, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = len;
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recvFrom(void* buffer, size_t len, Address::ptr from, int flags) {
    if(isConnected()) {
        socklen_t len = from->getAddrLen();
        return ::recvfrom(m_sock, buffer, len, flags, const_cast<sockaddr*>(from->getAddr()), &len);
    }
    return -1;
}

int Socket::recvFrom(iovec* buffers, size_t len, Address::ptr from, int flags) {
    if(isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = len;
        msg.msg_name = (sockaddr*)from->getAddr();
        msg.msg_namelen = from->getAddrLen();
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

Address::ptr Socket::getRemoteAddress() {
    if(m_remoteAddress) {
        return m_remoteAddress;
    }

    Address::ptr result;
    switch(m_family) {
    case AF_INET:
        result.reset(new IPv4Address);
        break;
    case AF_INET6:
        result.reset(new IPv6Address);
        break;
    case AF_UNIX:
        result.reset(new UnixAddress);
        break;
    default:
        result.reset(new UnkonwAddress(m_family));
        break;
    }

    socklen_t addrlen = result->getAddrLen();
    if(getpeername(m_sock, const_cast<sockaddr*>(result->getAddr()), &addrlen)) {
        FL_LOG_ERROR(syslog) << "getpeername error sock = " << m_sock
                             << " errno = " << errno << " errstr = " << strerror(errno);
        return Address::ptr(new UnkonwAddress(m_family));
    }
    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }
    m_remoteAddress = result;
    return m_remoteAddress;
}

Address::ptr Socket::getLocalAddress() {
    if(m_localAddress) {
        return m_localAddress;
    }

    Address::ptr result;
    switch(m_family) {
    case AF_INET:
        result.reset(new IPv4Address);
        break;
    case AF_INET6:
        result.reset(new IPv6Address);
        break;
    case AF_UNIX:
        result.reset(new UnixAddress);
        break;
    default:
        result.reset(new UnkonwAddress(m_family));
        break;
    }

    socklen_t addrlen = result->getAddrLen();
    if(getsockname(m_sock, const_cast<sockaddr*>(result->getAddr()), &addrlen)) {
        FL_LOG_ERROR(syslog) << "getsockname error sock = " << m_sock
                             << " errno = " << errno << " error info = " << strerror(errno);
        return Address::ptr(new UnkonwAddress(m_family));
    }
    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }
    m_localAddress = result;
    return m_localAddress;
}

bool Socket::isValid() const {
    return m_sock != -1;
}

int Socket::getError() {
    int error = 0;
    socklen_t len = sizeof(error);
    if(!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
        error = errno;
    }
    return error;
}

bool Socket::cancelRead() {
    return IOManager::GetThis()->cancelEvent(m_sock, IOManager::READ);
}

bool Socket::cancelWrite() {
    return IOManager::GetThis()->cancelEvent(m_sock, IOManager::WRITE);
}

bool Socket::cancelAccept() {
    return IOManager::GetThis()->cancelEvent(m_sock, IOManager::READ);
}

bool Socket::cancelAll() {
    return IOManager::GetThis()->cancelAll(m_sock);
}

std::ostream& Socket::dump(std::ostream& os) const {
    os << "[Socket sock = " << m_sock
       << " is_connected = " << m_isConnected
       << " family = " << m_family
       << " type = " << m_type
       << " protocol = " << m_protocol;
    if(m_localAddress) {
        os << " local_address = " << m_localAddress->toString();
    }
    if(m_remoteAddress) {
        os << " remote_address = " << m_remoteAddress->toString();
    }
    os << "]";
    return os;
}

std::string Socket::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

void Socket::initSock() {
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, val);
    if(m_type == SOCK_STREAM) {
        setOption(IPPROTO_TCP, TCP_NODELAY, val);
    }
}

void Socket::newSock() {
    m_sock = socket(m_family, m_type, m_protocol);
    if(FL_LICKLY(m_sock != -1)) {
        initSock();
    } else {
        FL_LOG_ERROR(syslog) << "socket(" << m_family
                             << ", " << m_type << ", " << m_protocol << ") errno = "
                             << errno << " error info = " << strerror(errno);
    }
}

bool Socket::init(int sock) {
    FdCtx::ptr ctx = fd_manager::GetInstance()->get(sock);
    if(ctx && ctx->isSocket() && !ctx->isClose()) {
        m_sock = sock;
        m_isConnected = true;
        initSock();
        getLocalAddress();
        getRemoteAddress();
        return true;
    }
    return false;
}

std::ostream& operator << (std::ostream& os, const Socket& sock) {
    return sock.dump(os);
}

}
