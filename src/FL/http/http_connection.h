#pragma once

#include "../socket_stream.h"
#include "../uri.h"
#include "http.h"
#include <map>

namespace FL {
namespace http {

struct HttpResult {
    typedef std::shared_ptr<HttpResult> ptr;
    HttpResult(int _result, HttpResponse::ptr _response, const std::string& _error)
        : result(_result)
        , response(_response)
        , error(_error) {}
    int result;
    HttpResponse::ptr response;
    std::string error;
};

class HttpConnection : public SocketStream {
  public:
    typedef std::shared_ptr<HttpConnection> ptr;
    typedef std::map<std::string, std::string> Map_t;

    HttpConnection(Socket::ptr sock, bool owner = true);
    HttpResponse::ptr recvResponse();
    int sendRequest(HttpRequest::ptr req);

    static HttpResult::ptr DoGet(const std::string& url
                                 , uint64_t timeout
                                 , const Map_t& headers = {}
                                 , const std::string body = "");

    static HttpResult::ptr DoGet(Uri::ptr uri
                                 , uint64_t timeout
                                 , const Map_t& headers = {}
                                 , const std::string body = "");

    static HttpResult::ptr DoPost(const std::string& url
                                  , uint64_t timeout
                                  , const Map_t& headers = {}
                                  , const std::string body = "");

    static HttpResult::ptr DoPost(Uri::ptr uri
                                  , uint64_t timeout
                                  , const Map_t& headers = {}
                                  , const std::string body = "");


    static HttpResult::ptr DoRequest(HttpMethod method
                                     , const std::string& url
                                     , uint64_t timeout
                                     , const Map_t& headers = {}
                                     , const std::string body = "");

    static HttpResult::ptr DoRequest(HttpMethod method
                                     , Uri::ptr uri
                                     , uint64_t timeout
                                     , const Map_t& headers = {}
                                     , const std::string body = "");

    static HttpResult::ptr DoRequest(HttpRequest::ptr req
                                     , Uri::ptr uri
                                     , uint64_t timeout);
  private:
};

}
}
