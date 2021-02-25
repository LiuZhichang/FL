#include "http.h"
#include <sstream>

namespace FL {
namespace http {


static std::map<std::string, HttpMethod> s_http_methods;
static std::map<std::string, HttpStatus> s_http_status;
static std::map<HttpMethod, const char*> s_http_methods_str;
static std::map<HttpStatus, const char*> s_http_status_str;

namespace {
struct __Initer__ {
    __Initer__() {
#define XX(num,name,string) s_http_methods[#name] = HttpMethod::name; \
		s_http_methods_str[HttpMethod::name] = #string;
        HTTP_METHOD_MAP(XX)
#undef XX

#define XX(code,name,desc) s_http_status[#name]= HttpStatus::name; \
		s_http_status_str[HttpStatus::name] = #desc;
        HTTP_STATUS_MAP(XX)
#undef XX
    }
};
static __Initer__ __initer__;
}

static const char* s_method_string[] = {
#define XX(num, name, string) #string,
    HTTP_METHOD_MAP(XX)
#undef XX
};

static const char* s_statu_string[] = {
#define XX(num, name, string) #string,
    HTTP_STATUS_MAP(XX)
#undef XX
};

HttpMethod StringToHttpMethod (const std::string& m) {
// TODO 后续改用map，下面的方法性能差
//#define XX(num,name,string) \
//	if(strcmp(#string,m.c_str()) == 0) { \
//		return HttpMethod::name; \
//	}
//	HTTP_METHOD_MAP(XX)
//#undef XX
    if(s_http_status.count(m)) {
        return s_http_methods[m];
    }
    return HttpMethod::INVALID_METHOD;
}

HttpStatus StringToHttpStatus (const std::string& m) {
    if(s_http_status.count(m)) {
        return s_http_status[m];
    }
    return HttpStatus::INVALID_STATU;
}

const char* HttpMethodToString(const HttpMethod&  m) {
    if(s_http_methods_str.count(m)) {
        return s_http_methods_str[m];
    }
    return "<method unkonw>";
}

HttpMethod CharsToHttpMethod(const char* m) {
#define XX(num, name, string) \
    if(strncmp(#string, m, strlen(#string)) == 0) { \
        return HttpMethod::name; \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}


const char* HttpStatusToString(const HttpStatus&  s) {
    if(s_http_status_str.count(s)) {
        return s_http_status_str[s];
    }
    return "<statu unkonw>";
}

bool CaseInsensitiveLess::operator()(const std::string& lhs, const std::string& rhs) const {
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

HttpRequest::HttpRequest(uint8_t version, bool close)
    : m_method(HttpMethod::GET)
    , m_version(version)
    , m_close(close)
    , m_path("/") {
}

std::string HttpRequest::getHeader(const std::string & key, const std::string & def) {
    auto it = m_headers.find(key);
    return it != m_headers.end() ? it->second : def;
}

std::string HttpRequest::getParam(const std::string& key, const std::string& def) {
    auto it = m_params.find(key);
    return it != m_params.end() ? it->second : def;
}

std::string HttpRequest::getCookie(const std::string& key, const std::string& def) {
    auto it = m_cookies.find(key);
    return it != m_cookies.end() ? it->second : def;
}

void HttpRequest::setHeader(const std::string& key, const std::string& val) {
    m_headers[key] = val;
}

void  HttpRequest::setParam(const std::string& key, const std::string& val) {
    m_params[key] = val;
}

void HttpRequest::setCookie(const std::string& key, const std::string& val) {
    m_cookies[key] = val;
}

void HttpRequest::delHeader(const std::string& key) {
    m_headers.erase(key);
}

void HttpRequest::delParam (const std::string& key) {
    m_params.erase(key);
}

void HttpRequest::delCookie(const std::string& key) {
    m_cookies.erase(key);
}

bool HttpRequest::hasHeader(const std::string& key, std::string* val) {
    auto it = m_headers.find(key);
    if(it == m_headers.end()) {
        return false;
    }
    if(val) {
        *val = it->second;
    }
    return true;
}

bool HttpRequest::hasParam (const std::string& key, std::string* val) {
    auto it = m_params.find(key);
    if(it == m_params.end()) {
        return false;
    }
    if(val) {
        *val = it->second;
    }
    return true;
}

bool HttpRequest::hasCookie(const std::string& key, std::string* val) {
    auto it = m_cookies.find(key);
    if(it == m_cookies.end()) {
        return false;
    }
    if(val) {
        *val = it->second;
    }
    return true;
}

std::ostream & HttpRequest::dump(std::ostream& os) const {
    os << HttpMethodToString(m_method) << " "
       << m_path
       << (m_query.empty() ? "" : "?")
       << m_query
       << (m_fragment.empty() ? "" : "#")
       << m_fragment
       << " HTTP/"
       << ((uint32_t)(m_version >> 4))
       << "."
       << ((uint32_t)(m_version & 0x0F))
       << "\r\n";

    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";

    for(auto& it : m_headers) {
        if(strcasecmp(it.first.c_str(), "connection") == 0) {
            continue;
        }
        os << it.first << ": " << it.second << "\r\n";
    }

    if(!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n"
           << m_body;
    } else {
        os << "\r\n";
    }

    return os;
}

std::string HttpRequest::toString() const {
std:
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

HttpResponse::HttpResponse(uint8_t version, bool close)
    : m_status(HttpStatus::OK)
    , m_version(version)
    , m_close(close) {
}

std::string HttpResponse::getHeader(const std::string& key, const std::string& def) const {
    auto it = m_headers.find(key);
    return it != m_headers.end() ? it->second : def;
}

void HttpResponse::setHeader(const std::string& key, const std::string& val) {
    m_headers[key] = val;
}

void HttpResponse::delHeader(const std::string& key) {
    m_headers.erase(key);
}

std::ostream& HttpResponse::dump(std::ostream& os) const {
    os << "HTTP/"
       << (uint32_t)(m_version >> 4)
       << "."
       << (uint32_t)(m_version & 0x0F)
       << " "
       << (uint32_t)m_status
       << " "
       << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason)
       << "\r\n";

    for(auto& it : m_headers) {
        if(strcasecmp(it.first.c_str(), "coonection") == 0) {
            continue;
        }
        os << it.first << ": " << it.second << "\r\n";
    }

    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";

    if(!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n"
           << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}

std::string HttpResponse::toString() const {
std:
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

std::ostream& operator << (std::ostream& os, const HttpRequest& req) {
    return req.dump(os);
}
std::ostream& operator << (std::ostream& os, const HttpResponse& rsp) {
    return rsp.dump(os);
}

}
}
