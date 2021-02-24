#pragma once

#include <memory>
#include "address.h"
#include "noncopyable.h"

namespace FL {

class Socket : public std::enable_shared_from_this<Socket>, FL::NonCopyable {
  public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;

    enum Type {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    enum Family {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX
    };

    static Socket::ptr CreateTCP(Address::ptr address);
    static Socket::ptr CreateUDP(Address::ptr address);

    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();

    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();

    static Socket::ptr CreateUnixTCPSocket();
    static Socket::ptr CreateUnixUDPSocket();

    Socket(int family, int type, int protocol = 0);
    virtual ~Socket();

    int64_t getSendTimeout();
    void setSendTimeout(uint64_t time);

    int64_t getRecvTimeout();
    void setRecvTimeout(uint64_t time);

    bool getOption(int level, int opt, void* result, socklen_t* len);
    template <typename T>
    bool getOption(int level, int opt, T& result) {
        size_t len = sizeof(T);
        return getOption(level, opt, &result, &len);
    }

    bool setOption(int level, int opt, const void* result, socklen_t len);
    template <typename T>
    bool setOption(int level, int opt, const T& value) {
        return setOption(level, opt, &value, sizeof(T));
    }

    virtual Socket::ptr accept();
    virtual bool bind(const Address::ptr addr);
    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
    virtual bool listen(int backlog = SOMAXCONN);
    virtual bool close();

    virtual int send(const void* buffer, size_t len, int flags = 0);
    virtual int send(const iovec* buffers, size_t len, int flags = 0);
    virtual int sendTo(const void* buffer, size_t len, const Address::ptr to, int flags = 0);
    virtual int sendTo(const iovec* buffers, size_t len, const Address::ptr to, int flags = 0);

    virtual int recv(void* buffer, size_t len, int flags = 0);
    virtual int recv(iovec* buffers, size_t len, int flags = 0);
    virtual int recvFrom(void* buffer, size_t len, Address::ptr from, int flags = 0);
    virtual int recvFrom(iovec* buffers, size_t len, Address::ptr from, int flags = 0);

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();

    int getFamily() const {
        return m_family;
    };

    int getType() const {
        return m_type;
    };

    int getProtocol() const {
        return m_protocol;
    };

    int getSokcet() const {
        return m_sock;
    };

    bool isConnected() const {
        return m_isConnected;
    };
    bool isValid() const;

    int getError();

    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();

    std::ostream& dump(std::ostream& os) const;
    std::string toString() const;
  private:
    void initSock();
    void newSock();
    bool init(int sock);
  private:
    int m_sock;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_isConnected;

    Address::ptr m_localAddress;
    Address::ptr m_remoteAddress;

};

std::ostream& operator << (std::ostream& os, const Socket& sock);

}
