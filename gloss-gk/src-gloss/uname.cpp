#include <sys/utsname.h>
#include <errno.h>
#include <cstring>

extern "C" int uname(struct utsname *buf)
{
    if(!buf)
    {
        errno = EFAULT;
        return -1;
    }

    strcpy(buf->machine, "arm");
    strcpy(buf->nodename, "gk");
    strcpy(buf->release, "0.1");
    strcpy(buf->sysname, "gkos");
    strcpy(buf->version, "0.1");

    return 0;
}
