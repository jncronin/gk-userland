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

int listen(int sockfd, int backlog)
{
    int ret, _errno;
    struct __syscall_listen_params p;
    p.sockfd = sockfd;
    p.backlog = backlog;
    __syscall(__syscall_listen, &ret, &p, &_errno);
    if(ret == -1)
        errno = _errno;
    return ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret, _errno;
    struct __syscall_accept_params p;
    p.sockfd = sockfd;
    p.addr = addr;
    p.addrlen = addrlen;
    __syscall(__syscall_accept, &ret, &p, &_errno);
    if(ret == -1)
        errno = _errno;
    else if(ret == -2)
    {
		struct WaitSimpleSignal_params wssp;
		while(1)
		{
			uint32_t wss;
			__syscall(WaitSimpleSignal, &wss, &wssp, NULL);
			if(wss)
				break;
		}
		if(wssp.ival1 == -1)
			errno = wssp.ival2;
		return wssp.ival1;
    }
    return ret;
}
