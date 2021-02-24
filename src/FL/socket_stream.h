#pragma once

#include "stream.h"
#include "socket.h"

namespace FL {

class SocketStream : public Stream {
  public:
    typedef std::shared_ptr<SocketStream> ptr;

    SocketStream(Socket::ptr sock, bool owner = true);
    virtual ~SocketStream();

    virtual int read(void* buffer, size_t length) override;
    virtual int read(ByteArray::ptr byte_arr, size_t length) override;

    virtual int write(const void* buffer, size_t length) override;
    virtual int write(ByteArray::ptr byte_arr, size_t length) override;
    virtual void close() override;

    Socket::ptr getSocket() const {
        return m_sock;
    }

	bool isConnected() const {
		return m_sock->isConnected();			
	}
  protected:
    Socket::ptr m_sock;
    bool		m_owner;
};

}
