#include "tcp_server.h"
#include "config.h"
#include "logmanager.h"
#include <cstring>

namespace FL {

static	FL::ConfigVar<uint64_t>::ptr g_tcp_serv_read_timeout =
    FL::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2), "tcp server read timeout");

static FL::Logger::ptr syslog = FL_SYS_LOG();

TcpServer::TcpServer(IOManager* worker, IOManager* accept_worker)
    : m_worker(worker)
    , m_accept_worker(accept_worker)
    , m_readTimeout(g_tcp_serv_read_timeout->getVal())
    , m_name("FL/1.0.0")
    , m_stop(true) {

}

TcpServer::~TcpServer() {
    for(auto& sock : m_socks) {
        sock->close();
    }
    m_socks.clear();
}

bool TcpServer::bind(Address::ptr addr) {
    Socket::ptr sock = Socket::CreateTCP(addr);
    if(!sock->bind(addr)) {
        FL_LOG_ERROR(syslog) << "bind failed errno = "
                             << errno << " error info = " << strerror(errno)
                             << " address = [" << addr->toString() << "]";
        return false;
    }
    if(!sock->listen()) {
        FL_LOG_ERROR(syslog) << "listen failed  errno = "
                             << errno << " error info = " << strerror(errno)
                             << " address = [" << addr->toString() << "]";
        return false;
    }
    m_socks.push_back(sock);
    FL_LOG_INFO(syslog) << "server bind success: " << *sock;
    return true;
//    std::vector<Address::ptr> addrs;
//    std::vector<Address::ptr> fails;
//    addrs.push_back(addr);
//    return bind(addrs, fails);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails) {
    for(auto& addr : addrs) {
        Socket::ptr sock = Socket::CreateTCP(addr);
        if(!sock->bind(addr)) {
            FL_LOG_ERROR(syslog) << "bind failed errno = "
                                 << errno << " error info = " << strerror(errno)
                                 << " address = [" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        if(!sock->listen()) {
            FL_LOG_ERROR(syslog) << "listen failed  errno = "
                                 << errno << " error info = " << strerror(errno)
                                 << " address = [" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }

    if(!fails.empty()) {
        m_socks.clear();
        return false;
    }

    for(auto& sock : m_socks) {
        FL_LOG_INFO(syslog) << "server bind success: " << *sock;
    }

    return true;

}

bool TcpServer::start() {
    if(!m_stop) {
        return true;
    }
    m_stop = false;

    for(auto& sock : m_socks) {
        m_accept_worker->schedule(std::bind(&TcpServer::startAccept
                                            , shared_from_this(), sock));
    }
    return true;
}

bool TcpServer::stop() {
    if(m_stop)
        return false;
    m_stop = true;
    auto self = shared_from_this();
    m_accept_worker->schedule([this, self]() {
        for(auto& sock : m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
    return true;
}

void TcpServer::handleClient(Socket::ptr client) {
    FL_LOG_INFO(syslog) << "handleClient: " << *client;
}

void TcpServer::startAccept(Socket::ptr sock) {
    while(!m_stop) {
        Socket::ptr client = sock->accept();
        if(client) {
            client->setRecvTimeout(m_readTimeout);
            m_worker->schedule(std::bind(&TcpServer::handleClient
                                         , shared_from_this(), client));
        } else {
            FL_LOG_ERROR(syslog) << "accept errno = " << errno
                                 << " errstr = " << strerror(errno);
        }
    }
}

}
