#pragma once

#include "http11_parser.h"
#include "httpclient_parser.h"

#include "http.h"

namespace FL {
namespace http {

class HttpRequestParser {
  public:
    typedef std::shared_ptr<HttpRequestParser> ptr;

    HttpRequestParser();

    size_t execute(char* data, size_t len, size_t off = 0);

    bool isFinish();
    bool hasError();

    void setError(int val) {
        m_error = val;
    }

    HttpRequest::ptr getData() const {
        return m_request;
    }
	
	uint64_t getContentLength();

    static uint64_t GetHttpRequestBufferSize();
    static uint64_t GetHttpResponsetMaxBodySize();

  private:
    http_parser		 m_parser;
    HttpRequest::ptr m_request;
    int				 m_error;
};

class HttpResponseParser {
  public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
    HttpResponseParser();

    size_t execute(char* data, size_t len, size_t off = 0);

    bool isFinish();
    bool hasError();

    void setError(int val) {
        m_error = val;
    }

    HttpResponse::ptr getData() const {
        return m_response;
    }
  private:
    httpclient_parser m_parser;
    HttpResponse::ptr m_response;
    int				  m_error;
};

}
}
