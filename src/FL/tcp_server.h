#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "iomanager.h"
#include "noncopyable.h"
#include "socket.h"
#include "address.h"

namespace FL {

class TcpServer : public std::enable_shared_from_this<TcpServer>, NonCopyable {
  public:
    typedef std::shared_ptr<TcpServer> ptr;

    TcpServer(IOManager* worker = IOManager::GetThis()
			,IOManager* accept_worker = IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs
                      , std::vector<Address::ptr>& fails);
    virtual bool start();
    virtual bool stop();

    uint64_t getReadTimeout() const {
        return m_readTimeout;
    }
    std::string getName() const {
        return m_name;
    }
    void setReadTimeout(uint64_t time) {
        m_readTimeout = time;
    }
    void setName(const std::string& name) {
        m_name = name;
    }

    bool isStop() const {
        return m_stop;
    }

  protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);
  private:
    std::vector<Socket::ptr>	m_socks;

    IOManager*	m_worker;
    IOManager*	m_accept_worker;
    uint64_t	m_readTimeout;
    bool		m_stop;
    std::string m_name;
};

}
