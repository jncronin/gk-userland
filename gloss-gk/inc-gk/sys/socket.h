#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include "_netinet_in.h"

#include <sys/uio.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>

struct sockaddr_storage
{
    uint8_t sa_len;
    sa_family_t ss_family;
    char sa_data[2];
    uint32_t sa_data2[6];
};

struct in6_addr
{
    union
    {
        uint8_t u6_addr8[16];
        uint16_t u6_addr16[8];
        uint32_t u6_addr32[4];
    } in6_u;
};

#define s6_addr in6_u.u6_addr8
#define s6_addr16 in6_u.u6_addr16
#define s6_addr32 in6_u.u6_addr32

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

#define INADDR_ANY      0

#ifdef __cplusplus
extern "C" {
#endif

int     accept(int, struct sockaddr *, socklen_t *);
int     bind(int, const struct sockaddr *, socklen_t);
int     connect(int, const struct sockaddr *, socklen_t);
int     getpeername(int, struct sockaddr *, socklen_t *);
int     getsockname(int, struct sockaddr *, socklen_t *);
int     getsockopt(int, int, int, void *, socklen_t *);
int     listen(int, int);
ssize_t recv(int, void *, size_t, int);
ssize_t recvfrom(int, void *, size_t, int,
        struct sockaddr *, socklen_t *);
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

#ifdef __cplusplus
}
#endif

#define SOL_SOCKET      1

#define SO_BROADCAST    2
#define SO_KEEPALIVE    3
#define SO_REUSEADDR    4
#define SO_SNDBUF       5
#define SO_RCVBUF       6
#define SO_ERROR        7
#define SO_RCVTIMEO     9
#define SO_SNDTIMEO     10

#define SOMAXCONN       32

#define TCP_NODELAY     100

#define IPV6_V6ONLY     200


#define SHUT_RD         1
#define SHUT_WR         2
#define SHUT_RDWR       3

#define MSG_OOB         1
#define MSG_PEEK        2
#define MSG_WAITALL     4
#define MSG_EOR         8
#define MSG_TRUNC       16
#define MSG_CTRUNC      32

#endif
