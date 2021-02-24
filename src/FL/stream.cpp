#include "stream.h"


namespace FL {

int Stream::readFixSize(void* buffer, size_t length) {
    if(length <= 0) {
        return -1;
    }
    size_t offset = 0;
    size_t left = length;
    while(left > 0) {
        size_t len = read((char*)buffer + offset, left);
        if(len <= 0) {
            return len;
        }
        offset += len;
        left -= len;
    }
    return length;
}

int Stream::readFixSize(ByteArray::ptr byte_arr, size_t length) {
    if(length <= 0) {
        return -1;
    }
    size_t left = length;
    while(left > 0) {
        size_t len = read(byte_arr, left);
        if(len <= 0) {
            return len;
        }
        left -= len;
    }
    return length;
}

int Stream::writeFixSize(const void* buffer, size_t length) {
    if(length <= 0) {
        return -1;
    }
    size_t offset = 0;
    size_t left = length;
    while(left > 0) {
        size_t len = write((const char*)buffer + offset, left);
        if(len <= 0) {
            return len;
        }
        offset += len;
        left -= len;
    }
    return length;
}

int Stream::writeFixSize(ByteArray::ptr byte_arr, size_t length) {
    if(length <= 0) {
        return -1;
    }
    size_t left = length;
    while(left > 0) {
        size_t len = write(byte_arr, left);
        if(len <= 0) {
            return len;
        }
        left -= len;
    }
    return length;
}

}
