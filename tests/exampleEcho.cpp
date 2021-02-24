#include "../src/FL/tcp_server.h"
#include "../src/FL/log.h"
#include "../src/FL/iomanager.h"
#include "../src/FL/bytearray.h"

auto lg = FL_SYS_LOG();

class EchoServer : public FL::TcpServer {
  public:
    EchoServer(int type);
    void handleClient(FL::Socket::ptr client) override;
  private:
    int type = 0;
};

int main(int argc, char** argv) {


    return 0;
}

EchoServer::EchoServer(int type)
    : type(type) {

}

void EchoServer::handleClient(FL::Socket::ptr client) {
    FL_LOG_INFO(lg) << "handle_client: " << *client;
    FL::ByteArray::ptr ba(new FL::ByteArray);
    while(true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, iovs.size());

        int rt = client->recv(&iovs[0], iovs.size());
    }
}
