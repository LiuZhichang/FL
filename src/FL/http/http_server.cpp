#include "http_server.h"
#include "http.h"
#include "../logmanager.h"

#include <cstring>

namespace FL {
namespace http {

static Logger::ptr syslog = FL_SYS_LOG();

HttpServer::HttpServer(bool keeplive
                       , IOManager* worker
                       , IOManager* accept_worker)
    : TcpServer(worker, accept_worker)
    , m_isKeeplive(keeplive) {
    m_dispatch.reset(new ServletDispatch());
}

void HttpServer::handleClient(Socket::ptr client) {
    http::HttpSession::ptr session(new HttpSession(client));
    do {
        auto req = session->recvRequest();
        if(!req) {
            FL_LOG_WARN(syslog) << "recv http request failed, errno = "
                                << errno << " error info = " << strerror(errno)
                                << " client: " << *client;
            break;
        }

        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeeplive));
        m_dispatch->handle(req, rsp, session);

        session->sendResponse(rsp);
    } while (m_isKeeplive);
    session->close();
}

}
}
