#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {

    sockaddr_in6 addr;

    inet_pton(AF_INET6, "127.0.0.1", &addr.sin6_addr);
    in6_addr ad = addr.sin6_addr;

    for(int i = 0 ; i < 16 ; ++i) {
        std::cout << ad.__in6_u.__u6_addr8[i] << std::endl;
   }



    return 0;
}
