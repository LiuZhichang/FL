#include "../src/FL/iomanager.h"
#include "../src/FL/logmanager.h"
#include "../src/FL/hook.h"
#include <string.h>
#include <arpa/inet.h>

FL::Logger::ptr g_logger = FL_LOG_ROOT();

void test_sleep() {
    FL::IOManager iom(1);
    iom.schedule([]() {
        sleep(2);
        FL_LOG_INFO(FL_LOG_ROOT()) << "sleep 2";
    });

    iom.schedule([]() {
        sleep(3);

        FL_LOG_INFO(FL_LOG_ROOT()) << "sleep 3";
    });
}

void test_sock() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "110.242.68.3", &addr.sin_addr.s_addr);

    FL_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    FL_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    FL_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    FL_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    FL_LOG_INFO(g_logger) << buff;
}

int main(int argc, char** argv) {
    //  test_sleep();
    FL::IOManager iom(1, true, "main");
    iom.schedule(test_sock);
    return 0;
}
