#include "../src/FL/tcp_server.h"
#include "../src/FL/iomanager.h"

auto lg = FL_SYS_LOG();

void run() {
    auto addr = FL::IPAddress::LookupAny("0.0.0.0:8033");
//    auto uaddr = FL::UnixAddress::ptr(new FL::UnixAddress("./unix_addr"));
    FL_LOG_INFO(lg) << *addr << " ---- "; //<< *uaddr;

    FL::TcpServer::ptr server(new FL::TcpServer);
    while(!server->bind(addr)) {
		FL_LOG_DEBUG(lg) << "bind failed";
		sleep(2);
	}
    server->start();
}

int main(int argc, char** argv) {

    FL::IOManager iom(2);
    iom.schedule(run);

    return 0;
}
