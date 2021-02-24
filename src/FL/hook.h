#pragma once

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "fd_manager.h"

namespace FL {
bool is_hook_enable();
void set_hook_enable(bool flag);
}

extern "C" {
//
// sleep
    typedef unsigned int (*sleep_fun) (unsigned int seconds);
    extern sleep_fun sleep_f;

    typedef int (*usleep_fun) (useconds_t us);
    extern usleep_fun usleep_f;

    typedef int (*nanosleep_fun) (const struct timespec* req, struct timespec* rem);
    extern nanosleep_fun nanosleep_f;

    typedef int (*socket_fun) (int __domain, int __type, int __protocol);
    extern socket_fun socket_f;

    typedef int (*connect_fun) (int __fd, const struct sockaddr *__addr, socklen_t __len);
    extern connect_fun connect_f;

    typedef int (*listen_fun) (int __fd, int __n);
    extern listen_fun listen_f;

    typedef int (*accept_fun) (int __fd, struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len);
    extern accept_fun accept_f;
//
// read
    typedef ssize_t (*read_fun) (int __fd, void *__buf, size_t __nbytes);
    extern read_fun read_f;

    typedef ssize_t (*readv_fun) (int __fd, const struct iovec *__iovec, int __count);
    extern readv_fun readv_f;

    typedef ssize_t (*recv_fun) (int __fd, void *__buf, size_t __n, int __flags);
    extern recv_fun recv_f;

    typedef ssize_t (*recvfrom_fun) (int __fd, void *__restrict __buf, size_t __n, int __flags, struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len);
    extern recvfrom_fun recvfrom_f;

    typedef ssize_t (*recvmsg_fun) (int __fd, struct msghdr *__message, int __flags);
    extern recvmsg_fun recvmsg_f;
//
// write
    typedef ssize_t (*write_fun) (int __fd, const void *__buf, size_t __n);
    extern write_fun write_f;

    typedef ssize_t (*writev_fun) (int __fd, const struct iovec *__iovec, int __count);
    extern writev_fun writev_f;

    typedef int (*send_fun) (int __fd, const void *__buf, size_t __n, int __flags);
    extern send_fun send_f;

    typedef ssize_t (*sendto_fun) (int __fd, const void *__buf, size_t __n, int __flags, const struct sockaddr *__addr, socklen_t __addr_len);
    extern sendto_fun sendto_f;

    typedef ssize_t (*sendmsg_fun) (int __fd, const struct msghdr *__message, int __flags);
    extern sendmsg_fun sendmsg_f;

    typedef int (*close_fun) (int __fd);
    extern close_fun close_f;

    typedef int (*fcntl_fun) (int __fd, int __cmd, ...);
    extern fcntl_fun fcntl_f;

    typedef int (*ioctl_fun) (int __fd, unsigned long __request, ...);
    extern ioctl_fun ioctl_f;

    typedef int (*setsockopt_fun) (int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen);
    extern setsockopt_fun setsockopt_f;

    typedef int (*getsockopt_fun) (int __fd, int __level, int __optname, void *__optval, socklen_t *__optlen);
    extern getsockopt_fun getsockopt_f;

    extern int connect_timeout(int __fd, const struct sockaddr *__addr, socklen_t __len, uint64_t timeout);
}
