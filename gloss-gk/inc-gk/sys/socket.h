#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

typedef unsigned int socklen_t;

#include <sys/uio.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>

typedef uint8_t sa_family_t;

struct sockaddr
{
    uint8_t sa_len;
    sa_family_t sa_family;
    char sa_data[14];
};

struct sockaddr_storage
{
    uint8_t sa_len;
    sa_family_t ss_family;
    char sa_data[2];
    uint32_t sa_data2[6];
};

struct in_addr
{
    uint32_t s_addr;
};

struct in6_addr
{
    unsigned char s6_addr[16];
};

struct sockaddr_in
{
    uint8_t sin_len;
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
#define SIN_ZERO_LEN 8
    char            sin_zero[SIN_ZERO_LEN];
};

struct sockaddr_in6 {
    uint8_t         sin6_len;
    sa_family_t     sin6_family;   /* AF_INET6 */
    in_port_t       sin6_port;     /* port number */
    uint32_t        sin6_flowinfo; /* IPv6 flow information */
    struct in6_addr sin6_addr;     /* IPv6 address */
    uint32_t        sin6_scope_id; /* Scope ID (new in Linux 2.4) */
};

struct msghdr
{
    void          *msg_name;
    socklen_t      msg_namelen;
    struct iovec  *msg_iov;
    int            msg_iovlen;
    void          *msg_control;
    socklen_t      msg_controllen;
    int            msg_flags;
};

#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3
#define SOCK_SEQPACKET  4
#define SOCK_RDM        5

#define AF_UNSPEC       0
#define AF_INET         2
#define AF_INET6        10
#define AF_UNIX         1

#define INADDR_ANY      0


int     accept(int, struct sockaddr *restrict, socklen_t *restrict);
int     bind(int, const struct sockaddr *, socklen_t);
int     connect(int, const struct sockaddr *, socklen_t);
int     getpeername(int, struct sockaddr *restrict, socklen_t *restrict);
int     getsockname(int, struct sockaddr *restrict, socklen_t *restrict);
int     getsockopt(int, int, int, void *restrict, socklen_t *restrict);
int     listen(int, int);
ssize_t recv(int, void *, size_t, int);
ssize_t recvfrom(int, void *restrict, size_t, int,
        struct sockaddr *restrict, socklen_t *restrict);
ssize_t recvmsg(int, struct msghdr *, int);
ssize_t send(int, const void *, size_t, int);
ssize_t sendmsg(int, const struct msghdr *, int);
ssize_t sendto(int, const void *, size_t, int, const struct sockaddr *,
        socklen_t);
int     setsockopt(int, int, int, const void *, socklen_t);
int     shutdown(int, int);
int     socket(int, int, int);
int     sockatmark(int);
int     socketpair(int, int, int, int[2]);

#define SOL_SOCKET      1

#define SO_BROADCAST    2
#define SO_KEEPALIVE    3
#define SO_REUSEADDR    4

#endif
