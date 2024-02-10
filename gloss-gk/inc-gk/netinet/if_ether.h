#ifndef _NETINET_IF_ETHER_H
#define _NETINET_IF_ETHER_H

#include <net/ethernet.h>
#include <net/if_arp.h>

struct        ether_arp {
        struct        arphdr ea_hdr;                /* fixed-size header */
        uint8_t arp_sha[ETH_ALEN];        /* sender hardware address */
        uint8_t arp_spa[4];                /* sender protocol address */
        uint8_t arp_tha[ETH_ALEN];        /* target hardware address */
        uint8_t arp_tpa[4];                /* target protocol address */
};
#define        arp_hrd        ea_hdr.ar_hrd
#define        arp_pro        ea_hdr.ar_pro
#define        arp_hln        ea_hdr.ar_hln
#define        arp_pln        ea_hdr.ar_pln
#define        arp_op        ea_hdr.ar_op

/*
 * Macro to map an IP multicast address to an Ethernet multicast address.
 * The high-order 25 bits of the Ethernet address are statically assigned,
 * and the low-order 23 bits are taken from the low end of the IP address.
 */
#define ETHER_MAP_IP_MULTICAST(ipaddr, enaddr) \
        /* struct in_addr *ipaddr; */ \
        /* u_char enaddr[ETH_ALEN];           */ \
{ \
        (enaddr)[0] = 0x01; \
        (enaddr)[1] = 0x00; \
        (enaddr)[2] = 0x5e; \
        (enaddr)[3] = ((u_int8_t *)ipaddr)[1] & 0x7f; \
        (enaddr)[4] = ((u_int8_t *)ipaddr)[2]; \
        (enaddr)[5] = ((u_int8_t *)ipaddr)[3]; \
}

#endif
