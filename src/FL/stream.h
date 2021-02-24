#pragma once

#include <memory>

#include "bytearray.h"

namespace FL {

class Stream {
  public:
    typedef std::shared_ptr<Stream> ptr;

    virtual ~Stream(){};

    virtual int read(void* buffer, size_t length) = 0;
    virtual int read(ByteArray::ptr byte_arr, size_t length) = 0;
    virtual int readFixSize(void* buffer, size_t length);
    virtual int readFixSize(ByteArray::ptr byte_arr, size_t length);

    virtual int write(const void* buffer, size_t length) = 0;
    virtual int write(ByteArray::ptr byte_arr, size_t length) = 0;
    virtual int writeFixSize(const void* buffer, size_t length);
    virtual int writeFixSize(ByteArray::ptr byte_arr, size_t length);

    virtual void close() = 0;

  private:
};


}