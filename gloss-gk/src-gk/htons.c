#include <arpa/inet.h>

uint16_t htons(uint16_t hostshort)
{
    return ((hostshort >> 8) & 0xff) | ((hostshort & 0xff) << 8);
}

uint16_t ntohs(uint16_t hostshort)
{
    return ((hostshort >> 8) & 0xff) | ((hostshort & 0xff) << 8);
}

uint32_t htonl(uint32_t hostlong)
{
    uint32_t b1 = hostlong & 0xff;
    uint32_t b2 = (hostlong >> 8) & 0xff;
    uint32_t b3 = (hostlong >> 16) & 0xff;
    uint32_t b4 = (hostlong >> 24) & 0xff;
    return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
}

uint32_t ntohl(uint32_t hostlong)
{
    uint32_t b1 = hostlong & 0xff;
    uint32_t b2 = (hostlong >> 8) & 0xff;
    uint32_t b3 = (hostlong >> 16) & 0xff;
    uint32_t b4 = (hostlong >> 24) & 0xff;
    return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
}
