#include "http_connection.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
#include "http_parser.h"

#include "../ScopeGuard.hpp"
#include <cstring>
#include <string>
#include <strings.h>

#include "../logmanager.h"

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

    auto& client_parser = parser->getParser();
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

HttpResult::ptr HttpConnection::DoGet(const std::string& url
                                      , uint64_t timeout
                                      , const Map_t& headers
                                      , const std::string body) {
    Uri::ptr uri = Uri::Create(url);
    if(!uri) {
        return HttpResult::ptr (new HttpResult(HttpResult::Result::INVALID_URL
                                               , nullptr, "invlid url: " + url));
    }
    return DoGet(uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri
                                      , uint64_t timeout
                                      , const Map_t& headers
                                      , const std::string body) {
    return DoRequest(HttpMethod::GET, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(const std::string& url
                                       , uint64_t timeout
                                       , const Map_t& headers
                                       , const std::string body) {
    Uri::ptr uri = Uri::Create(url);
    if(!uri) {
        return HttpResult::ptr (new HttpResult(HttpResult::Result::INVALID_URL
                                               , nullptr, "invlid url: " + url));
    }
    return DoPost(uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri
                                       , uint64_t timeout
                                       , const Map_t& headers
                                       , const std::string body) {
    return DoRequest(HttpMethod::POST, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method
        , const std::string& url
        , uint64_t timeout
        , const Map_t& headers
        , const std::string body) {
    Uri::ptr uri = Uri::Create(url);
    if(!uri) {
        return HttpResult::ptr (new HttpResult(HttpResult::Result::INVALID_URL
                                               , nullptr, "invlid url: " + url));
    }
    return DoRequest(method, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method
        , Uri::ptr uri
        , uint64_t timeout
        , const Map_t& headers
        , const std::string body) {

    HttpRequest::ptr req(new HttpRequest);
    req->setPath(uri->getPath());
    req->setMethod(method);
    req->setQuery(uri->getQuery());
    req->setFragment(uri->getFragment());
    bool has_host = false;
    for(auto& item : headers) {
        if(strcasecmp(item.first.c_str(), "connection") == 0) {
            if(strcasecmp(item.second.c_str(), "keep-alive") == 0) {
                req->setClose(false);
            }
            continue;
        }

        if(!has_host && strcasecmp(item.first.c_str(), "host") == 0) {
            has_host = !item.second.empty();
        }
        req->setHeader(item.first, item.second);
    }
    if(!has_host) {
        req->setHeader("Host", uri->getHost());
    }
    req->setBody(body);
    return DoRequest(req, uri, timeout);
}

HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req
        , Uri::ptr uri
        , uint64_t timeout) {
    Address::ptr addr = uri->createAddress();
    if(!addr) {
        return HttpResult::ptr(new HttpResult(HttpResult::Result::INVALID_HOST
                                              , nullptr
                                              , "invalid host: " + uri->getHost()));
    }
    Socket::ptr sock = Socket::CreateTCP(addr);
    if(!sock) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::CREATE_SOCKET_ERROR
                                               , nullptr
                                               , "create scoket fail: " + addr->toString()));
    }
    if(!sock->connect(addr)) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::CONNECT_FAIL
                                               , nullptr
                                               , "connect fail: " + addr->toString()));
    }
    sock->setRecvTimeout(timeout);

    HttpConnection::ptr conn(new HttpConnection(sock));
    int rt = conn->sendRequest(req);
    if(rt == 0) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::SEND_FAIL
                                               , nullptr
                                               , "send request fail: " + addr->toString()));
    } else if(rt < 0) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::SEND_ERROR
                                               , nullptr
                                               , "send request error, errno: " + std::to_string(errno)
                                               + "  error info = " + std::string(strerror(errno))));
    }

    HttpResponse::ptr rsp = conn->recvResponse();
    if(!rsp) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::TIMEOUT
                                               , nullptr
                                               , "recv response timeout: " + addr->toString()
                                               + "timeout_ms:" + std::to_string(timeout)));
    }


    return HttpResult::ptr (new HttpResult(HttpResult::Result::OK, rsp, "ok"));
}

HttpConnectionPool::HttpConnectionPool(const std::string& host
                                       , const std::string& vhost
                                       , uint32_t port
                                       , uint32_t max_size
                                       , uint32_t max_alive_time
                                       , uint32_t max_request)
    : m_host(host)
    , m_vhost(vhost)
    , m_port(port)
    , m_maxSize(max_size)
    , m_maxAliveTime(max_alive_time)
    , m_maxRequest(max_request) {

}

HttpConnection::ptr HttpConnectionPool::getConnection() {
    uint64_t now_ms = FL::UT::GetCurrentMs();
    Mutex_t::Lock lock(m_mutex);

    while(!m_conns.empty()) {
        auto conn = m_conns.front();
        if(!conn->isConnected()) {
            m_conns.pop_front();
            continue;
        }
    }
}

HttpResult::ptr HttpConnectionPool::doGet(const std::string& url
        , uint64_t timeout
        , const Map_t& headers
        , const std::string body) {

}

HttpResult::ptr HttpConnectionPool::doGet(Uri::ptr uri
        , uint64_t timeout
        , const Map_t& headers
        , const std::string body) {
    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?")
       << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#")
       << uri->getFragment();
    return doGet(ss.str(), timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::doPost(const std::string& url
        , uint64_t timeout
        , const Map_t& headers
        , const std::string body) {

}

HttpResult::ptr HttpConnectionPool::doPost(Uri::ptr uri
        , uint64_t timeout
        , const Map_t& headers
        , const std::string body) {

    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?")
       << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#")
       << uri->getFragment();
    return doPost(ss.str(), timeout, headers, body);
}


HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method
        , const std::string& url
        , uint64_t timeout
        , const Map_t& headers
        , const std::string body) {

    HttpRequest::ptr req(new HttpRequest);
    req->setPath(url);
    req->setMethod(method);
    bool has_host = false;
    for(auto& item : headers) {
        if(strcasecmp(item.first.c_str(), "connection") == 0) {
            if(strcasecmp(item.second.c_str(), "keep-alive") == 0) {
                req->setClose(false);
            }
            continue;
        }

        if(!has_host && strcasecmp(item.first.c_str(), "host") == 0) {
            has_host = !item.second.empty();
        }
        req->setHeader(item.first, item.second);
    }
    if(!has_host) {
        if(m_vhost.empty()) {
            req->setHeader("Host", m_host);
        } else {
            req->setHeader("Host", m_vhost);
        }
    }
    req->setBody(body);
    return doRequest(req, timeout);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method
        , Uri::ptr uri
        , uint64_t timeout
        , const Map_t& headers
        , const std::string body) {

    std::stringstream ss;
    ss << uri->getPath()
       << (uri->getQuery().empty() ? "" : "?")
       << uri->getQuery()
       << (uri->getFragment().empty() ? "" : "#")
       << uri->getFragment();
    return doRequest(method, ss.str(), timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpRequest::ptr req
        , uint64_t timeout) {
    auto conn = getConnection();
    if(!conn) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::GET_CONECTION_FAIL
                                               , nullptr
                                               , "pool host: " +  m_host + " port: " + std::to_string(m_port)));
    }
    Socket::ptr sock = conn->getSocket();
    if(!sock) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::POOL_INVALID_CONECTION
                                               , nullptr
                                               , "pool host: " +  m_host + " port: " + std::to_string(m_port)));
    }
    sock->setRecvTimeout(timeout);
    int rt = conn->sendRequest(req);
    if(rt == 0) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::SEND_FAIL
                                               , nullptr
                                               , "send request fail"));
    } else if(rt < 0) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::SEND_ERROR
                                               , nullptr
                                               , "send request error, errno: " + std::to_string(errno)
                                               + "  error info = " + std::string(strerror(errno))));
    }

    HttpResponse::ptr rsp = conn->recvResponse();
    if(!rsp) {
        return 	HttpResult::ptr(new HttpResult(HttpResult::Result::TIMEOUT
                                               , nullptr
                                               , "recv response timeout, timeout_ms:" + std::to_string(timeout)));
    }


    return HttpResult::ptr (new HttpResult(HttpResult::Result::OK, rsp, "ok"));
}

void HttpConnectionPool::ReleasePtr(HttpConnection * ptr, HttpConnectionPool * pool) {

}


}
}
