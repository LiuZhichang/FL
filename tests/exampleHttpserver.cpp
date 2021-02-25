#include "../src/FL/http/http_server.h"

auto lg = FL_SYS_LOG();

void task() {
    FL::http::HttpServer::ptr server(new FL::http::HttpServer);
    FL::Address::ptr addr = FL::Address::LookupAnyIPAddress("0.0.0.0:8020");
    bool rt = server->bind(addr);
    if(!rt) {
        sleep(2);
    }
    auto sd = server->getServletDispath();
    sd->addServlet("/FL/XX", [](FL::http::HttpRequest::ptr req
                             , FL::http::HttpResponse::ptr rsp
    , FL::http::HttpSession::ptr session) {
        rsp->setBody(req->toString());
        return 0;
    });
    server->start();
    sd->addGlobServlet("/FL/*", [](FL::http::HttpRequest::ptr req
                             , FL::http::HttpResponse::ptr rsp
    , FL::http::HttpSession::ptr session) {
        rsp->setBody(rsp->toString());
        return 0;
    });
    server->start();
}

int main(int argc, char** argv) {

    FL::IOManager iom(2);

    iom.schedule(task);

    return 0;
}
