#pragma once

#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "http.h"
#include "http_session.h"
#include "../mutex.h"

namespace FL {
namespace http {

class Servlet {
  public:
    typedef std::shared_ptr<Servlet> ptr;

    Servlet(const std::string& name)
        : m_name(name) {}
    virtual ~Servlet() {}
    virtual int32_t handle(HttpRequest::ptr requeset
                           , HttpResponse::ptr response
                           , HttpSession::ptr session) = 0;
    const std::string& getName() const {
        return m_name;
    }
  protected:
    std::string m_name;
};

class FunctionServlet : public Servlet {
  public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function < uint32_t(HttpRequest::ptr requeset
                                     , HttpResponse::ptr response
                                     , HttpSession::ptr session) > Callback_t;

    FunctionServlet(Callback_t cb);
    virtual int32_t handle(HttpRequest::ptr requeset
                           , HttpResponse::ptr response
                           , HttpSession::ptr session) override;
  private:
    Callback_t m_cb;
};

class ServletDispatch : public Servlet {
  public:
    typedef std::shared_ptr<ServletDispatch> ptr;
    typedef RWMutex Mutex_t;

    ServletDispatch();
    virtual int32_t handle(HttpRequest::ptr requeset
                           , HttpResponse::ptr response
                           , HttpSession::ptr session) override;

    void addServlet(const std::string& uri, Servlet::ptr slt);
    void addServlet(const std::string& uri, FunctionServlet::Callback_t cb);
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);
    void addGlobServlet(const std::string& uri, FunctionServlet::Callback_t cb);

    void delServlet(const std::string& uri);
    void delGlobServlet(const std::string& uri);

    Servlet::ptr getDefault() const {
        return m_default;
    }
    void setDefault(Servlet::ptr servlet) {
        m_default = servlet;
    }

    Servlet::ptr getServlet(const std::string& uri);
    Servlet::ptr getGlobServlet(const std::string& uri);

    Servlet::ptr getMatchServlet(const std::string& uri);
  private:
    Mutex_t m_mutex;
    std::unordered_map<std::string, Servlet::ptr> m_datas;
    std::vector<std::pair<std::string, Servlet::ptr>> m_globs;
    Servlet::ptr m_default;
};

class NotFoundServlet : public Servlet {
  public:
    typedef std::shared_ptr<NotFoundServlet> ptr;

    NotFoundServlet();
    virtual int32_t handle(HttpRequest::ptr requeset
                           , HttpResponse::ptr response
                           , HttpSession::ptr session) override;
};

}
}
