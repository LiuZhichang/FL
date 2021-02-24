#include "socket_stream.h"
#include <vector>

namespace FL {

SocketStream::SocketStream(Socket::ptr sock, bool owner)
    : m_sock(sock)
    , m_owner(owner) {
}

SocketStream::~SocketStream() {
    if(m_owner && m_sock) {
        m_sock->close();
    }
}

int SocketStream::read(void* buffer, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    return m_sock->recv(buffer, length);
}

int SocketStream::read(ByteArray::ptr byte_arr, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    if(!byte_arr->getWriteBuffers(iovs, length)) {
        return -1;
    }
    int result = m_sock->recv(&iovs[0], iovs.size());
    if(result) {
        byte_arr->setPosition(byte_arr->getPosition() + result);
    }
    return result;
}

int SocketStream::write(const void* buffer, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    return m_sock->send(buffer, length);
}

int SocketStream::write(ByteArray::ptr byte_arr, size_t length) {
    if(!isConnected()) {
        return -1;
    }
    std::vector<iovec> iovs;
    if(!byte_arr->getReadBuffers(iovs, length)) {
        return -1;
    }
    int result = m_sock->send(&iovs[0], iovs.size());
    if(result) {
        byte_arr->setPosition(byte_arr->getPosition() + result);
    }
    return result;
}

void SocketStream::close() {
	if(m_sock) {
		m_sock->close();
	}
}

}
