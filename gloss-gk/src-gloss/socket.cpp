#include <errno.h>
#include "syscalls.h"
#include "deferred.h"

extern "C"
{

int socket(int domain, int type, int protocol)
{
    struct __syscall_socket_params p;
    p.domain = domain;
    p.type = type;
    p.protocol = protocol;
    return deferred_call(__syscall_socket, &p);
}

int bind(int sockfd, const struct sockaddr *addr,
    socklen_t addrlen)
{
    struct __syscall_bind_params p;
    p.sockfd = sockfd;
    p.addr = addr;
    p.addrlen = addrlen;
    return deferred_call(__syscall_bind, &p);
}

int listen(int sockfd, int backlog)
{
    struct __syscall_listen_params p;
    p.sockfd = sockfd;
    p.backlog = backlog;
    return deferred_call(__syscall_listen, &p);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    struct __syscall_accept_params p;
    p.sockfd = sockfd;
    p.addr = addr;
    p.addrlen = addrlen;
    return deferred_call(__syscall_accept, &p);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
    const sockaddr *dest_addr, socklen_t addrlen)
{
    __syscall_sendto_params p
    { sockfd, buf, len, flags, dest_addr, addrlen };
    return deferred_call(__syscall_sendto, &p);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    return sendto(sockfd, buf, len, flags, nullptr, 0);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
    struct sockaddr *src_addr, socklen_t *addrlen)
{
    __syscall_recvfrom_params p
    { sockfd, buf, len, flags, src_addr, addrlen };
    return deferred_call(__syscall_recvfrom, &p);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    return recvfrom(sockfd, buf, len, flags, nullptr, nullptr);
}

}
