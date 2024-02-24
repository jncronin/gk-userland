#include <arpa/inet.h>

uint16_t htons(uint16_t hostshort)
{
    return ((hostshort >> 8) & 0xff) | ((hostshort & 0xff) << 8);
}
