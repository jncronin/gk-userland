#ifndef _NETINET_IN_H
#define _NETINET_IN_H

#include <inttypes.h>
#include <sys/socket.h>

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

#define IN6_IS_ADDR_V4MAPPED(a)     0

#endif
