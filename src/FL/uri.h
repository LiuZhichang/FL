#pragma once

#include <memory>
#include <string>

#include "address.h"

namespace FL {

class Uri {
  public:
    typedef std::shared_ptr<Uri> ptr;
    Uri();

    static Uri::ptr Create(const std::string& uri);

    const std::string& getScheme() const {
        return m_scheme;
    }
    const std::string& getUserinfo() const {
        return m_userinfo;
    }
    const std::string& getHost() const {
        return m_host;
    }
    const std::string& getPath() const;

    const std::string& getQuery() const {
        return m_query;
    }
    const std::string& getFragment() const {
        return m_fragment;
    }

    int16_t getPort() const;

    void setScheme(const std::string& scheme) {
        m_scheme = scheme;
    }
    void setUserinfo(const std::string& userinfo) {
        m_userinfo = userinfo;
    }
    void setHost(const std::string& host) {
        m_host = host;
    }
    void setPath(const std::string& path) {
        m_path = path;
    }
    void setQuery(const std::string& query) {
        m_query = query;
    }
    void setFragment(const std::string& fragment) {
        m_fragment = fragment;
    }
    void setPort(int16_t port) {
        m_port = port;
    }

    Address::ptr createAddress() const;
    std::ostream& dump(std::ostream& os) const;
    std::string toString() const;
  private:
    bool isDefaultPort() const;

  private:
    std::string m_scheme;
    std::string m_userinfo;
    std::string	m_host;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    int16_t		m_port;
};

std::ostream& operator << (std::ostream& os, const Uri& uri);

}
