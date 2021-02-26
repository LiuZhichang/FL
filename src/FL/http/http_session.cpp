#include "http_session.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
#include "http_parser.h"
#include "../ScopeGuard.hpp"

namespace FL {
namespace http {

HttpSession::HttpSession(Socket::ptr sock, bool owner)
    : SocketStream(sock, owner) {
}

HttpRequest::ptr HttpSession::recvRequest() {
    HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buf_size = HttpRequestParser::GetHttpRequestBufferSize();
    char* buffer = new char[buf_size];
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
        size_t nparser = parser->execute(buffer, len);
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

    int64_t length = parser->getContentLength();
    if(length > 0) {
        std::string body;
        body.resize(length);

        int len = 0;
        if(length >= offset) {
            memcpy(&body[0], buffer, offset);
            body.append(buffer, offset);
        } else {
            memcpy(&body[0], buffer, length);
            body.append(buffer, length);
        }
        length -= offset;
        if(length > 0) {
            if(readFixSize(&body[len], length) <= 0) {
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }
    return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp) {
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size());
}


}
}
