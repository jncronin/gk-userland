#ifndef _ARPA_INET_H
#define _ARPA_INET_H

#include <stdint.h>

#ifdef __cplusplus
#define restrict
extern "C" {
#endif

#include <netinet/in.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

in_addr_t    inet_addr(const char *);
char        *inet_ntoa(struct in_addr);
const char  *inet_ntop(int, const void *restrict, char *restrict,
                 socklen_t);
int          inet_pton(int, const char *restrict, void *restrict);
int inet_aton(const char *cp, struct in_addr *inp);

#ifdef __cplusplus
}
#endif

#endif
