#include "http_servlet.h"
#include "resource/rsp_page.hpp"

#include "../logmanager.h"

#include <fnmatch.h>

namespace FL {
namespace http {

FunctionServlet::FunctionServlet(Callback_t cb)
    : Servlet("FunctionServlet")
    , m_cb(cb) {

}

int32_t FunctionServlet::handle(HttpRequest::ptr requeset
                                , HttpResponse::ptr response
                                , HttpSession::ptr session) {
    return m_cb(requeset, response, session);
}

ServletDispatch::ServletDispatch()
    : Servlet("ServletDispatch") {
    m_default.reset(new NotFoundServlet);
}

int32_t ServletDispatch::handle(HttpRequest::ptr requeset
                                , HttpResponse::ptr response
                                , HttpSession::ptr session) {
    auto slt = getMatchServlet(requeset->getPath());
    if(slt) {
        slt->handle(requeset, response, session);
    } else {
        return -1;
    }
    return 0;
}

void ServletDispatch::addServlet(const std::string& uri, Servlet::ptr slt) {
    Mutex_t::WriteLock lock(m_mutex);
    m_datas[uri] = slt;
}

void ServletDispatch::addServlet(const std::string& uri, FunctionServlet::Callback_t cb) {
    Mutex_t::WriteLock lock(m_mutex);
    m_datas[uri].reset(new FunctionServlet(cb));
}

void ServletDispatch::addGlobServlet(const std::string& uri, Servlet::ptr slt) {
    Mutex_t::WriteLock lock(m_mutex);
    for(auto it = m_globs.begin() ; it != m_globs.end() ; ++it) {
        if(it->first == uri) {
            m_globs.erase(it);
            break;
        }
    }
    m_globs.push_back(std::make_pair(uri, slt));
}

void ServletDispatch::addGlobServlet(const std::string& uri, FunctionServlet::Callback_t cb) {
    return addGlobServlet(uri, FunctionServlet::ptr(new FunctionServlet(cb)));
}

void ServletDispatch::delServlet(const std::string& uri) {
    Mutex_t::WriteLock lock(m_mutex);
    m_datas.erase(uri);
}

void ServletDispatch::delGlobServlet(const std::string& uri) {
    Mutex_t::WriteLock lock(m_mutex);
    for(auto it = m_globs.begin() ; it != m_globs.end() ; ++it) {
        if(it->first == uri) {
            m_globs.erase(it);
            break;
        }
    }
}

Servlet::ptr ServletDispatch::getServlet(const std::string& uri) {
    Mutex_t::ReadLock lock(m_mutex);
    auto it = m_datas.find(uri);
    return it == m_datas.end() ? nullptr : it->second;
}

Servlet::ptr ServletDispatch::getGlobServlet(const std::string& uri) {
    Mutex_t::ReadLock lock(m_mutex);
    for(auto it = m_globs.begin() ; it != m_globs.end() ; ++it) {
        if(it->first == uri) {
            return it->second;
        }
    }
    return nullptr;
}

Servlet::ptr ServletDispatch::getMatchServlet(const std::string& uri) {
    Mutex_t::ReadLock lock(m_mutex);
    auto it = m_datas.find(uri);
    if(it != m_datas.end()) {
        return it->second;
    }

    for(auto it = m_globs.begin() ; it != m_globs.end() ; ++it) {
        if(!fnmatch(it->first.c_str(),uri.c_str(),0)) {
            return it->second;
        }
    }
    return m_default;
}

NotFoundServlet::NotFoundServlet()
    : Servlet("NotFoundServlet") {
  //  GetContent() = "<html><head><title>404 Not Found"
  //                 "</title></head><body><center><h1>404 Not Found</h1></center>"
  //                 "<hr><center> FL 1.0 </center></body></html>";
}

int32_t NotFoundServlet::handle(HttpRequest::ptr requeset
                                , HttpResponse::ptr response
                                , HttpSession::ptr session) {
    response->setStatus(HttpStatus::NOT_FOUND);
    response->setHeader("Content-Type", "text/html");
    response->setHeader("Server", "FL/1.0.0");
    response->setBody(RSP_BODY);
    return 0;
}

}
}
