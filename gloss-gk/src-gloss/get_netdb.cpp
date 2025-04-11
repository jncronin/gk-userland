#include <netdb.h>
#include <string.h>
#include <errno.h>

extern "C" struct servent *getservent(void)
{
    return nullptr;
}

extern "C" struct servent *getservbyname(const char *name, const char *proto)
{
    return nullptr;
}

extern "C" struct servent *getservbyport(int port, const char *proto)
{
    return nullptr;
}

extern "C" void setservent(int stayopen)
{

}

extern "C" void endservent(void)
{

}

extern "C" int gethostname(char *name, size_t len)
{
    strncpy(name, "gk", len);
    return 0;
}

static char ntoa_buf[32];
extern "C" char *inet_ntoa(in_addr in)
{
    snprintf(ntoa_buf, 31, "%u.%u.%u.%u",
        in.s_addr & 0xff,
        (in.s_addr >> 8) & 0xff,
        (in.s_addr >> 16) & 0xff,
        (in.s_addr >> 24) & 0xff);
    return ntoa_buf;
}

extern "C" int inet_aton(const char *cp, in_addr *inp)
{
    if(!cp)
    {
        errno = EINVAL;
        return -1;
    }
    if(!inp)
    {
        errno = EINVAL;
        return -1;
    }
    unsigned int b1, b2, b3, b4;
    sscanf(cp, "%u.%u.%u.%u", &b1, &b2, &b3, &b4);

    inp->s_addr = (b1 & 0xff) |
        ((b2 & 0xff) << 8) |
        ((b3 & 0xff) << 16) |
        ((b4 & 0xff) << 24);
    return 0;
}

extern "C" in_addr_t inet_addr(const char *cp)
{
    in_addr ia;
    if(inet_aton(cp, &ia) == 0)
    {
        return ia.s_addr;
    }
    else
    {
        return (in_addr_t)-1;
    }
}

extern "C" hostent *gethostbyaddr(const void *addr, socklen_t len, int type)
{
    return nullptr;
}

extern "C" hostent *gethostbyname(const char *name)
{
    return nullptr;
}
