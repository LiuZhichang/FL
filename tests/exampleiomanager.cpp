#include "../src/FL/iomanager.h"
#include "../src/FL/logmanager.h"
#include "../src/FL/timer.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <string.h>

FL::Logger::ptr g_logger = FL_LOG_ROOT();

int sock = 0;

void test_fiber() {
    FL_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //FL::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "110.242.68.3", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        FL_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        FL::IOManager::GetThis()->addEvent(sock, FL::IOManager::READ, []() {
            FL_LOG_INFO(g_logger) << "read callback";
        });
        FL::IOManager::GetThis()->addEvent(sock, FL::IOManager::WRITE, []() {
            FL_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            FL::IOManager::GetThis()->cancelEvent(sock, FL::IOManager::READ);
            close(sock);
        });
    } else {
        FL_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    FL::IOManager iom(2, false, "thread");
    iom.schedule(&test_fiber);
}

FL::Timer::ptr s_timer;
void test_timer() {
    FL::IOManager iom(2);
    s_timer = iom.addTimer(1000, []() {
        static int i = 0;
        FL_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 3) {
            s_timer->reset(4000, true);
            FL_LOG_DEBUG(g_logger) << "---------------- reset 4000 ----------------";
        }
    }, true);
}

int main(int argc, char** argv) {
    FL::Thread::SetName("main");
    test1();
    //test_timer();

    return 0;
}
