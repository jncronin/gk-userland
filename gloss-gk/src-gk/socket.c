#include <sys/socket.h>
#include <errno.h>
#include "syscalls.h"

int socket(int domain, int type, int protocol)
{
    int ret, _errno;
    struct __syscall_socket_params p;
    p.domain = domain;
    p.type = type;
    p.protocol = protocol;
    __syscall(__syscall_socket, &ret, &p, &_errno);
    if(ret == -1)
        errno = _errno;
    return ret;
}

int bind(int sockfd, const struct sockaddr *addr,
    socklen_t addrlen)
{
    int ret, _errno;
    struct __syscall_bind_params p;
    p.sockfd = sockfd;
    p.addr = (void *)addr;
    p.addrlen = addrlen;
    __syscall(__syscall_bind, &ret, &p, &_errno);
    if(ret == -1)
        errno = _errno;
    return ret;
}
