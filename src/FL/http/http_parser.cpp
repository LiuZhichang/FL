#include "http_parser.h"
#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
#include <cstring>

#include "../logmanager.h"
#include "../config.h"

namespace FL {
namespace http {

static FL::Logger::ptr syslog = FL_SYS_LOG();

static FL::ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
    FL::Config::Lookup("http.request.buffer_size", 4 * 1024ul, "http request buffer size");

static FL::ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
    FL::Config::Lookup("http.request.max_body_size", 64 * 1024 * 1024ul, "http request max body size");

static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_max_body_size = 0;

uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
    return s_http_request_buffer_size;
}
uint64_t HttpRequestParser::GetHttpResponsetMaxBodySize() {
    return s_http_request_max_body_size;
}

namespace {
struct __RequestSizeIniter__ {
    __RequestSizeIniter__() {
        s_http_request_buffer_size = g_http_request_buffer_size->getVal();
        s_http_request_max_body_size = g_http_request_max_body_size->getVal();
        g_http_request_buffer_size->addListener(
        [](const uint64_t& old_v, const uint64_t& new_v) {
            s_http_request_buffer_size = new_v;
        }
        );
        g_http_request_max_body_size->addListener(
        [](const uint64_t& old_v, const uint64_t& new_v) {
            s_http_request_max_body_size = new_v;
        }
        );
    }
};

static __RequestSizeIniter__ __ini__;
}

void on_request_http_field(void* data, const char* field, size_t flen, const char* value, size_t vlen) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    if(flen == 0) {
        FL_LOG_WARN(syslog) << "invalid http request field length == 0";
        parser->setError(1002);
        return;
    }
    parser->getData()->setHeader(std::string(field, flen), std::string(value, vlen));
}
void on_request_method(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    HttpMethod m = CharsToHttpMethod(at);

    if(m == HttpMethod::INVALID_METHOD) {
        FL_LOG_WARN(syslog) << "invalid http request method: "
                            << std::string(at, length);
        parser->setError(1000);
        return;
    }
    parser->getData()->setMethod(m);
}
void on_request_uri(void* data, const char* at, size_t length) {

}
void on_request_fragment(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setFragment(std::string(at, length));
}
void on_request_path(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setPath(std::string(at, length));
}
void on_request_query(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setQuery(std::string(at, length));
}
void on_request_version(void* data, const char* at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    uint8_t version = 0;
    if(strncmp(at, "HTTP/1.1", length) == 0) {
        version = 0x11;
    } else if(strncmp(at, "HTTP/1.0", length) == 0) {
        version = 0x10;
    } else {
        FL_LOG_WARN(syslog) << "invalid http request version: "
                            << std::string(at, length);
        parser->setError(10001);
        return;
    }
    parser->getData()->setVersion(version);
}
void on_request_header_done(void* data, const char* at, size_t length) {

}

void on_response_http_field(void* data, const char* field, size_t flen, const char* value, size_t vlen) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    if(flen == 0) {
        FL_LOG_WARN(syslog) << "invalid http response field length == 0";
        parser->setError(1002);
        return;
    }
    parser->getData()->setHeader(std::string(field, flen), std::string(value, vlen));
}
void on_response_reason(void* data, const char* at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getData()->setReason(std::string(at, length));
}
void on_response_status(void* data, const char* at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    HttpStatus status = (HttpStatus)(atoi(at));
    parser->getData()->setStatus(status);
}
void on_response_chunk(void* data, const char* at, size_t length) {

}
void on_response_version(void* data, const char* at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    uint8_t version = 0;
    if(strncmp(at, "HTTP/1.1", length) == 0) {
        version = 0x11;
    } else if(strncmp(at, "HTTP/1.0", length) == 0) {
        version = 0x10;
    } else {
        FL_LOG_WARN(syslog) << "invalid http response version: "
                            << std::string(at, length);
        parser->setError(10001);
        return;
    }
    parser->getData()->setVersion(version);
}
void on_response_header_done(void* data, const char* at, size_t length) {

}
void on_response_last_chunk(void* data, const char* at, size_t length) {

}

HttpRequestParser::HttpRequestParser()
    : m_error(0) {
    m_request.reset(new HttpRequest);

    http_parser_init(&m_parser);

    m_parser.http_field		= on_request_http_field;
    m_parser.request_method = on_request_method;
    m_parser.request_uri	= on_request_uri;
    m_parser.fragment		= on_request_fragment;
    m_parser.request_path	= on_request_path;
    m_parser.query_string	= on_request_query;
    m_parser.http_version	= on_request_version;
    m_parser.header_done	= on_request_header_done;
    m_parser.data			= this;
}

size_t HttpRequestParser::execute(char* data, size_t len, size_t off) {
    size_t offset = http_parser_execute(&m_parser, data, len, off);
    if(offset == -1) {
        FL_LOG_WARN(syslog) << "invalid request: " << std::string(data, len);
    } else if(offset != 1) {
        memmove(data, data + off + offset, (len - off - offset));
    }
    return offset;
}

bool HttpRequestParser::isFinish() {
    return http_parser_finish(&m_parser);
}

bool HttpRequestParser::hasError() {
    return m_error || http_parser_has_error(&m_parser);
}
uint64_t HttpRequestParser::getContentLength() {
	return m_request->getHaderAs<uint64_t>("content-length",0);
}

HttpResponseParser::HttpResponseParser()
    : m_error(0) {
    m_response.reset(new HttpResponse);

    httpclient_parser_init(&m_parser);
    m_parser.http_field		= on_response_http_field;
    m_parser.reason_phrase	= on_response_reason;
    m_parser.status_code  	= on_response_status;
    m_parser.chunk_size	  	= on_response_chunk;
    m_parser.http_version 	= on_response_version;
    m_parser.header_done  	= on_response_header_done;
    m_parser.last_chunk	  	= on_response_last_chunk;
    m_parser.data			= this;
}

size_t HttpResponseParser::execute(char* data, size_t len, size_t off) {
    size_t offset = httpclient_parser_execute(&m_parser, data, len, off);
    if(offset == -1) {
        FL_LOG_WARN(syslog) << "invalid response: " << std::string(data, len);
    } else if(offset != 1) {
        memmove(data, data + offset, (len - offset));
    }
    return offset;
}

bool HttpResponseParser::isFinish() {
    return httpclient_parser_finish(&m_parser);
}

bool HttpResponseParser::hasError() {
    return m_error || httpclient_parser_has_error(&m_parser);
}

}
}
