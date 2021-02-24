#pragma once

#include <byteswap.h>
#include <stdint.h>
#include <endian.h>
#include <type_traits>

#define FL_LITTLE_ENDIAN 1
#define FL_BIG_ENDIAN	 2

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T val) {
    return (T)bswap_64((uint64_t)val);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T val) {
    return (T)bswap_32((uint32_t)val);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T val) {
    return (T)bswap_16((uint16_t)val);
}

#if BYTE_ORDER == BIG_ENDIAN
#define FL_BYTE_ORDER FL_BIG_ENDIAN
#else
#define FL_BYTE_ORDER FL_LITTLE_ENDIAN
#endif

#if FL_BYTE_ORDER == FL_BIG_ENDIAN
template <class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

template <class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else
template <class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

template <class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif
