#include "../src/FL/socket.h"
#include "../src/FL/address.h"
#include "../src/FL/iomanager.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

auto lg = FL_LOG_ROOT();

void test_sock() {
    FL::IPAddress::ptr addr = FL::Address::LookupAnyIPAddress("www.baidu.com");
    addr->setPort(80);
//    sockaddr_in ad;
//    memset(&ad, 0, sizeof(ad));
//    ad.sin_family = AF_INET;
//    inet_pton(AF_INET, "110.242.68.3", &ad.sin_addr.s_addr);
//    FL::IPAddress::ptr addr(new FL::IPv4Address(ad));
//    addr->setPort(80);


//    FL::IPAddress::ptr addr = FL::IPv4Address::Create("110.242.68.3", 80);
    if(addr) {
        FL_LOG_INFO(lg) << "get address: " << addr->toString();
    } else {
        FL_LOG_ERROR(lg) << "get address failed";
        return;
    }
//    int sock = socket(AF_INET, SOCK_STREAM, 0);
//    FL_LOG_INFO(lg) << "sock: " << sock;
//    int rt = connect(sock, addr->getAddr(), addr->getAddrLen());
//    if(!rt) {
//        FL_LOG_ERROR(lg) << "connect " << addr->toString() << " failed";
//    } else {
//        FL_LOG_INFO(lg) << "connect " << addr->toString() << " connected";
//    }

    FL::Socket::ptr sock = FL::Socket::CreateTCP(addr);
    FL_LOG_INFO(lg) << "address: " << addr->toString();
    if(!sock->connect(addr)) {
        FL_LOG_ERROR(lg) << "connect " << addr->toString() << " failed";
    } else {
        FL_LOG_INFO(lg) << "connect " << addr->toString() << " connected";
    }

    const char buf[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buf, sizeof(buf));
    if(rt <= 0) {
        FL_LOG_INFO(lg) << "send failed rt = " << rt;
        return;
    }

    std::string buffer;
    buffer.resize(4096);
    rt = sock->recv(&buffer[0], buffer.size());

    if(rt <= 0) {
        FL_LOG_INFO(lg) << "recv failed rt = " << rt;
        return;
    }

    buffer.resize(rt);
    FL_LOG_INFO(lg) << buffer;

}

int main(int argc, char** argv) {

    FL::IOManager iom(1, true, "test");
    iom.schedule(test_sock);

    return 0;
}
