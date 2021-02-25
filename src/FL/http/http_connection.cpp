#include "http_connection.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
#include "http_parser.h"
#include "../ScopeGuard.hpp"
#include <cstring>

namespace FL {
namespace http {

HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    : SocketStream(sock, owner) {
}

HttpResponse::ptr HttpConnection::recvResponse() {
    HttpResponseParser::ptr parser(new HttpResponseParser);
    uint64_t buf_size = HttpResponseParser::GetHttpResponseBufferSize();
    char* buffer = new char[buf_size + 1];
    ON_SCOPE_EXIT {
        delete[] buffer;
    };

    int offset = 0;
    do {
        int len = read(buffer + offset, buf_size - offset);
        if(len <= 0) {
            close();
            return nullptr;
        }

        len += offset;
        buffer[len] = '\0';
        size_t nparser = parser->execute(buffer, len, false);
        if(parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparser;
        if(offset == buf_size) {
            close();
            return nullptr;
        }

        if(parser->isFinish()) {
            break;
        }
    } while(true);

    auto client_parser = parser->getParser();
    std::string body;
    if(client_parser.chunked) {
        int len = offset;
        do {
            bool begin = true;
            do {
                if(!begin || len == 0) {
                    int result = read(buffer + len, buf_size - len);
                    if(result <= 0) {
                        close();
                        return nullptr;
                    }
                    len += result;
                }
                buffer[len] = '\0';
                size_t nparse = parser->execute(buffer, len, true);
                if(parser->hasError()) {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if(len == buf_size) {
                    close();
                    return nullptr;
                }
                begin = false;
            } while (!parser->isFinish());

            if(client_parser.content_len + 2 <= len) {
                body.append(buffer, client_parser.content_len);
                memmove(buffer, buffer + client_parser.content_len + 2, len - client_parser.content_len - 2);
                len -= client_parser.content_len + 2;
            } else {
                body.append(buffer, len);
                int left = client_parser.content_len - len + 2;
                while(left > 0) {
                    int result = read(buffer, left > buf_size ? buf_size : left);
                    if(result <= 0) {
                        close();
                        return nullptr;
                    }
                    body.append(buffer, result);
                    left -= result;
                }
                body.resize(body.size() - 2);
                len = 0;
            }
        } while (client_parser.chunks_done);
    } else {
        int64_t length = parser->getContentLength();
        if(length > 0) {
            std::string body;
            body.resize(length);

            int len = 0;
            if(length >= offset) {
                memcpy(&body[0], buffer, offset);
            } else {
                memcpy(&body[0], buffer, length);
            }
            length -= offset;
            if(length > 0) {
                if(readFixSize(&body[len], length) <= 0) {
                    return nullptr;
                }
            }
            parser->getData()->setBody(body);
        }
    }
    return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size());
}


}
}
