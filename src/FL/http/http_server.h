#pragma once

#include "../tcp_server.h"
#include "http_session.h"
#include "http_servlet.h"

namespace FL {
namespace http {

class HttpServer : public TcpServer {
  public:
    typedef std::shared_ptr<HttpServer> ptr;

    HttpServer(bool keeplive = false
			, IOManager* worker = IOManager::GetThis()
			, IOManager* accept_worker =  IOManager::GetThis());
	
	ServletDispatch::ptr getServletDispath() const {
		return m_dispatch;
	}
	void setServletDispath(ServletDispatch::ptr servd) {
		m_dispatch = servd;
	}

  protected:
    virtual void handleClient(Socket::ptr client) override;

  private:
    bool m_isKeeplive;
	ServletDispatch::ptr m_dispatch;
};

}
}
