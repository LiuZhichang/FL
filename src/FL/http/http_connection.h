#pragma once

#include "../socket_stream.h"
#include "../uri.h"
#include "../mutex.h"

#include "http.h"

#include <list>
#include <map>
#include <vector>
#include <atomic>

namespace FL {
namespace http {

struct HttpResult {
    typedef std::shared_ptr<HttpResult> ptr;
    enum class Result {
        OK = 0x00,
        INVALID_URL				= 0x01,
        INVALID_HOST			= 0x02,
        CONNECT_FAIL			= 0x03,
        SEND_FAIL				= 0x04,
        SEND_ERROR				= 0x05,
        TIMEOUT					= 0x06,
        CREATE_SOCKET_ERROR		= 0x07,
        GET_CONECTION_FAIL		= 0x08,
        POOL_INVALID_CONECTION	= 0x09
    };
    HttpResult(Result _result, HttpResponse::ptr _response, const std::string& _error)
        : result(Result::OK)
        , response(_response)
        , error(_error) {}

    Result result;
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

class HttpConnectionPool {
  public:
    typedef std::shared_ptr<HttpConnectionPool> ptr;
    typedef std::map<std::string, std::string> Map_t;
    typedef Mutex Mutex_t;

    HttpConnectionPool(const std::string& host
                       , const std::string& vhost
                       , uint32_t port
                       , uint32_t max_size
                       , uint32_t max_alive_time
                       , uint32_t max_request);
    HttpConnection::ptr getConnection();

    HttpResult::ptr doGet(const std::string& url
                          , uint64_t timeout
                          , const Map_t& headers = {}
                          , const std::string body = "");

    HttpResult::ptr doGet(Uri::ptr uri
                          , uint64_t timeout
                          , const Map_t& headers = {}
                          , const std::string body = "");

    HttpResult::ptr doPost(const std::string& url
                           , uint64_t timeout
                           , const Map_t& headers = {}
                           , const std::string body = "");

    HttpResult::ptr doPost(Uri::ptr uri
                           , uint64_t timeout
                           , const Map_t& headers = {}
                           , const std::string body = "");


    HttpResult::ptr doRequest(HttpMethod method
                              , const std::string& url
                              , uint64_t timeout
                              , const Map_t& headers = {}
                              , const std::string body = "");

    HttpResult::ptr doRequest(HttpMethod method
                              , Uri::ptr uri
                              , uint64_t timeout
                              , const Map_t& headers = {}
                              , const std::string body = "");

    HttpResult::ptr doRequest(HttpRequest::ptr req
                              , uint64_t timeout);
  private:
    static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);
  private:
    std::string m_host;
    std::string m_vhost;
    int32_t		m_port;
    int32_t		m_maxSize;
    int32_t		m_maxAliveTime;
    int32_t		m_maxRequest;

    Mutex_t		m_mutex;
    std::list<HttpConnection*> m_conns;
    std::atomic<int32_t>	   m_total = {0};
};

}
}
