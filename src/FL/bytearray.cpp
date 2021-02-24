#include "bytearray.h"
#include "endian.h"
#include "logmanager.h"
#include "ScopeGuard.hpp"
#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace FL {

ByteArray::Node::Node(size_t size)
    : ptr(new char[size])
    , size(size)
    , next(nullptr) {
}

ByteArray::Node::Node()
    : ptr(nullptr)
    , size(0)
    , next(nullptr) {
}

ByteArray::Node::~Node() {
    if(ptr) {
        delete[] ptr;
    }
}

ByteArray::ByteArray(size_t base_size)
    : m_base_size(base_size)
    , m_position(0)
    , m_capacity(base_size)
    , m_size(0)
    , m_endian(FL_BIG_ENDIAN)
    , m_root(new Node(base_size))
    , m_cur(m_root) {
}

ByteArray::~ByteArray() {
    Node* tmp = m_root;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
}

void ByteArray::writeFint8 (int8_t val) {
    write(&val, sizeof(val));
}

void ByteArray::writeFuint8(int8_t val) {
    write(&val, sizeof(val));
}

#define xx(val) \
if (m_endian != FL_BYTE_ORDER) { \
	val = byteswap(val); \
} \
write(&val, sizeof(val)); \

void ByteArray::writeFint16(int16_t val) {
    xx(val)
}

void ByteArray::writeFuint16(int16_t val) {
    xx(val)
}

void ByteArray::writeFint32 (int32_t val) {
    xx(val)
}

void ByteArray::writeFuint32(int32_t val) {
    xx(val)
}

void ByteArray::writeFint64 (int64_t val) {
    xx(val)
}
#undef xx

void ByteArray::writeFuint64(int64_t val) {
    if (m_endian != FL_BYTE_ORDER) {
        val = byteswap(val);
    }
    write(&val, sizeof(val));
}

static uint32_t EncodeZigzag32(const int32_t& val) {
    if(val < 0) {
        return ((uint32_t)(val)) * 2 - 1;
    } else {
        return val * 2;
    }
}

static uint32_t EncodeZigzag64(const int64_t& val) {
    if(val < 0) {
        return ((uint64_t)(val)) * 2 - 1;
    } else {
        return val * 2;
    }
}

static int32_t DecodeZigzag32(const uint32_t& val) {
    return (val >> 2) ^ -(val & 1);
}

static int64_t DecodeZigzag64(const uint32_t& val) {
    return (val >> 2) ^ -(val & 1);
}

void ByteArray::writeInt32  (int32_t val) {
    writeUint32(EncodeZigzag32(val));
}

void ByteArray::writeUint32 (int32_t val) {
    uint8_t tmp[10];
    uint8_t i = 0;
    while(val >= 0x80) {
        tmp[i++] = (val & 0x7F | 0x80);
        val >>= 7;
    }
    tmp[i++] = val;
    write(tmp, i);
}

void ByteArray::writeInt64  (int64_t val) {
    writeUint64(EncodeZigzag64(val));
}

void ByteArray::writeUint64 (int64_t val) {
    uint8_t tmp[5];
    uint8_t i = 0;
    while(val >= 0x80) {
        tmp[i++] = (val & 0x7F | 0x80);
        val >>= 7;
    }
    tmp[i++] = val;
    write(tmp, i);
}

void ByteArray::writeFloat  (float val) {
    uint32_t v;
    memcpy(&v, &val, sizeof(val));
    writeFuint32(v);
}

void ByteArray::writeDouble (double val) {
    uint64_t v;
    memcpy(&v, &val, sizeof(val));
    writeFuint64(v);
}

#define xx(byte_n,val) \
	writeFuint##byte_n(val.size()); \
	write(val.c_str(),val.size()); \

void ByteArray::writeStringF16 (const std::string& val) {
    xx(16, val)
}

void ByteArray::writeStringF32 (const std::string& val) {
    xx(32, val)
}

void ByteArray::writeStringF64 (const std::string& val) {
    xx(64, val)
}

void ByteArray::ByteArray::writeStringVint(const std::string& val) {
    xx(64, val)
}

#undef xx

void ByteArray::ByteArray::writeStringWithoutLength(const std::string& val) {
    write(val.c_str(), val.size());
}

int8_t ByteArray::readFint8   () {
    int8_t v;
    read(&v, sizeof(v));
    return v;
}

uint8_t ByteArray::readFuint8 () {
    uint8_t v;
    read(&v, sizeof(v));
    return v;
}

#define xx(type) \
	type v; \
	read(&v,sizeof(v)); \
	if(m_endian == FL_BYTE_ORDER) { \
		return v;	\
	} else {	\
		return byteswap(v); \
	}

int16_t ByteArray::readFint16 () {
    xx(int16_t)
}

uint16_t ByteArray::ByteArray::readFuint16() {
    xx(uint16_t)
}

int32_t ByteArray::readFint32 () {
    xx(int32_t)
}

uint32_t ByteArray::ByteArray::readFuint32() {
    xx(uint32_t)
}

int64_t ByteArray::readFint64 () {
    xx(int64_t)
}

uint64_t ByteArray::ByteArray::readFuint64() {
    xx(uint64_t)
}

#undef xx

int32_t ByteArray::readInt32   (const int32_t& val) {
    return DecodeZigzag32(readFint32());
}

uint32_t ByteArray::readUint32 (const int32_t& val) {
    uint32_t result = 0;
    for(int i = 0 ; i < 32 ; i += 7) {
        uint8_t b = readFuint8();
        if(b < 0x80) {
            result |= ((uint32_t)b) << i;
            break;
        } else {
            result |= (((uint32_t)(b & 0x7F)) << i);
        }
    }
    return result;
}

int64_t ByteArray::readInt64   (const int64_t& val) {
    return DecodeZigzag64(readFint64());
}

uint64_t ByteArray::readUint64 (const int64_t& val) {
    uint64_t result = 0;
    for(int i = 0 ; i < 64 ; i += 7) {
        uint8_t b = readFuint8();
        if(b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        } else {
            result |= (((uint64_t)(b & 0x7F)) << i);
        }
    }
    return result;
}

float ByteArray::readFloat   () {
    uint32_t v = readFuint32();
    float val;
    memcpy(&val, &v, sizeof(v));

    return val;
}

double ByteArray::readDouble () {
    uint64_t v = readFuint32();
    double val;
    memcpy(&val, &v, sizeof(v));

    return val;
}

std::string ByteArray::readStringF16 () {
    uint16_t len = readFuint16();
    std::string buf;
    buf.resize(len);
    read(&buf[0], len);
    return buf;
}

std::string ByteArray::readStringF32 () {
    uint32_t len = readFuint32();
    std::string buf;
    buf.resize(len);
    read(&buf[0], len);
    return buf;
}

std::string ByteArray::readStringF64 () {
    uint64_t len = readFuint64();
    std::string buf;
    buf.resize(len);
    read(&buf[0], len);
    return buf;
}

std::string ByteArray::ByteArray::readStringVint() {
    uint64_t len = readFuint64();
    std::string buf;
    buf.resize(len);
    read(&buf[0], len);
    return buf;
}

void ByteArray::clear() {
    m_position = m_size = 0;
    m_capacity = m_base_size;
    Node* tmp = m_root->next;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete tmp;
    }
    m_cur = m_root;
    m_root->next = nullptr;
}

void ByteArray::write(const void* buf, size_t size) {
    if(size == 0) {
        return;
    }
    addCapacity(size);

    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;

    while(size > 0) {
        if(ncap >= size) {
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, size);
            if(m_cur->size == (npos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            npos = 0;
        }
    }

    if(m_position > m_size) {
        m_size = m_position;
    }
}

void ByteArray::read (void* buf, size_t size) {
    if(size > getReadSize()) {
        throw std::out_of_range("not enough len");
    }

    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;
    while(size > 0) {
        if(ncap >= size) {
            memcpy((char*)buf + bpos, (char*)m_cur->ptr + npos, size);
            if(m_cur->size == npos + size) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }
}

void ByteArray::read (void* buf, size_t size, size_t position) const {
    if(size > getReadSize()) {
        throw std::out_of_range("not enough len");
    }

    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;
    Node* cur = m_cur;
    while(size > 0) {
        if(ncap >= size) {
            memcpy((char*)buf + bpos, (char*)m_cur->ptr + npos, size);
            if(cur->size == npos + size) {
                cur = cur->next;
            }
            position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, ncap);
            position += ncap;
            bpos += ncap;
            size -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
    }
}

void ByteArray::setPosition(size_t pos) {
    if(pos > m_capacity) {
        throw std::out_of_range("set_position out of range");
    }
    m_position = pos;

	if(m_position > m_size) {
		m_size = m_position;
	}

    m_cur = m_root;
    while(pos >= m_cur->size) {
        pos -= m_cur->size;
        m_cur = m_cur->next;
    }
    if(pos == m_cur->size) {
        m_cur = m_cur->next;
    }
}

bool ByteArray::writeToFile (const std::string& name) const {
    std::fstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if(!ofs) {
        FL_LOG_ERROR(FL_SYS_LOG()) << "writeToFile name = " << name
                                   << " error , erorno = " << errno << " errstr = " << strerror(errno);
        return false;
    }
    int64_t read_size = getReadSize();
    int64_t pos = m_position;
    Node* cur = m_cur;
    while(read_size > 0) {
        int diff = pos % m_base_size;
        int64_t len = (read_size > (int64_t)m_base_size ? m_base_size : read_size - diff);

        ofs.write(cur->ptr + diff, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    ofs.close();
    return true;
}

bool ByteArray::readFromFile(const std::string& name) {
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if(!ifs) {
        FL_LOG_ERROR(FL_SYS_LOG()) << "readFromFile name = " << name
                                   << " error , erorno = " << errno << " errstr = " << strerror(errno);
        return false;
    }

    char* buf = new char[m_base_size];
    ON_SCOPE_EXIT {
        delete [] buf;
    };

    while(!ifs.eof()) {
        ifs.read(buf, m_base_size);
        write(buf, ifs.gcount());
    }
    ifs.close();
    return true;
}

bool ByteArray::isLittleEdian() const {
    return m_endian == FL_LITTLE_ENDIAN;
}

void ByteArray::setIsLittleEndian(bool val) {
    if(val) {
        m_endian = FL_LITTLE_ENDIAN;
    } else {
        m_endian = FL_BIG_ENDIAN;
    }
}

void ByteArray::addCapacity(size_t size) {
    if(size == 0) {
        return;
    }
    int old_cap = getCapacity();
    if(old_cap > size) {
        return;
    }

    size = size - old_cap;
    size_t count = (size / m_base_size) + ((size % m_base_size) ? 1 : 0);
    Node* tmp = m_root;
    while(tmp->next) {
        tmp = tmp->next;
    }

    Node* first = nullptr;
    for(size_t i = 0 ; i < count; ++i) {
        tmp->next = new Node(m_base_size);
        if(first == nullptr) {
            first = tmp->next;
        }
        tmp = tmp->next;
        m_capacity += m_base_size;
    }

    if(old_cap == 0) {
        m_cur = first;
    }

}

std::string ByteArray::toString() const {
    std::string str;
    str.resize(getReadSize());
    if(str.empty()) {
        return str;
    }
    read(&str[0], str.size(), m_position);
    return str;
}

std::string ByteArray::toHexString() const {
    std::string str = toString();
    std::stringstream ss;

    for(size_t i = 0 ; i < str.size() ; ++i) {
        if(i > 0 && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex
           << (int)(uint8_t)str[i] << " ";
    }
    return ss.str();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const {
    if(len == 0) {
        return 0;
    }
    len = len > getReadSize() ? getReadSize() : len;

    uint64_t size = len;

    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;

    Node* cur = m_cur;
    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const {
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) {
        return 0;
    }

    uint64_t size = len;

    size_t npos = position % m_base_size;

    size_t count = position / m_base_size;
    Node* cur = m_root;
    while(count > 0) {
        cur = cur->next;
        --count;
    }

    size_t ncap = cur->size - npos;
    struct iovec iov;

    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) {
    if(len == 0) {
        return 0;
    }
    addCapacity(len);
    uint64_t size = len;
    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node* cur = m_cur;
    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;

            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

}
