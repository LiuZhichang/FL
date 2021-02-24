#pragma once

#include "../tcp_server.h"
#include "http_session.h"

namespace FL {
namespace http {

class HttpServer : public TcpServer {
  public:
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keeplive = false
			, IOManager* worker = IOManager::GetThis()
			, IOManager* accept_worker =  IOManager::GetThis());

  protected:
    virtual void handleClient(Socket::ptr client) override;

  private:
    bool m_isKeeplive;
};

}
}
