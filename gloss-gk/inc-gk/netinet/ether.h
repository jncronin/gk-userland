#ifndef _NETINET_ETHER_H
#define _NETINET_ETHER_H

#include <netinet/if_ether.h>
char *ether_ntoa (const struct ether_addr *__addr);
struct ether_addr *ether_aton (const char *__asc);
int ether_ntohost (char *__hostname, const struct ether_addr *__addr);
int ether_hostton (const char *__hostname, struct ether_addr *__addr);
int ether_line (const char *__line, struct ether_addr *__addr,
		       char *__hostname);

#endif
