#include "../src/FL/http/http_parser.h"
#include "../src/FL/logmanager.h"
#include <string>
#include <iostream>

auto lg = FL_LOG_ROOT();

char request_data[] = "GET / HTTP/1.1\r\n"
                      "Host: www.baidu.com\r\n"
                      "Content-Length: 10\r\n\r\n"
                      "1234567890";

void test() {
    FL::http::HttpRequestParser parser;
    size_t size = parser.execute(request_data, sizeof(request_data) - 1);
    FL_LOG_INFO(lg) << "execute result = "  << size
                     << " has error = " << parser.hasError()
                     << " is_finished = " << parser.isFinish();

    FL_LOG_INFO(lg) << "\n" << parser.getData()->toString();
}

int main(int argc, char** argv) {
    test();
    return 0;
}
