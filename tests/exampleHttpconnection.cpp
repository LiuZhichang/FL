#include "../src/FL/http/http_connection.h"
#include "../src/FL/logmanager.h"
#include "../src/FL/iomanager.h"

auto lg = FL_SYS_LOG();

void task() {
    FL_LOG_INFO(lg) << "task start";
    FL::Address::ptr address = FL::Address::LookupAnyIPAddress("www.sylar.top:80");
    if(!address) {
        FL_LOG_INFO(lg) << "get addr failed";
        return;
    } else {
        FL_LOG_INFO(lg) << "get addr: " << *address;
    }
    FL::Socket::ptr sock = FL::Socket::CreateTCP(address);
    bool result = sock->connect(address);
    if(!result) {
        FL_LOG_INFO(lg) << "connect " << *address << " failed";
        return;
    } else {
        FL_LOG_INFO(lg) << "connect " << *address << " success";
    }

    FL::http::HttpConnection::ptr conn(new FL::http::HttpConnection(sock));
    FL::http::HttpRequest::ptr req(new FL::http::HttpRequest);

    FL_LOG_INFO(lg) << "req: " << std::endl << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp) {
        FL_LOG_INFO(lg) << "recv response error";
        return;
    }

    FL_LOG_INFO(lg) << "rsp: " << std::endl << *rsp;
}

int main(int argc, char** argv) {

    FL::IOManager iom(2);
    iom.schedule(task);

    return 0;
}
