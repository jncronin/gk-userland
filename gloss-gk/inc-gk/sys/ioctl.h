#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

int ioctl(int fd, unsigned long request, ...);

#include "_gk_ioctls.h"

#define _IO(type,nr,argtype)    (0xff000000 | (((type) & 0xff) << 16) | ((nr) & 0xffff))
#define _IOR(type,nr,argtype) _IO(type,nr,argtype)
#define _IOW(type,nr,argtype) _IO(type,nr,argtype)
#define _IOWR(type,nr,argtype) _IO(type,nr,argtype)
#define _IOC(dir,group,nr,size) _IO(group,nr,0)
	
#ifdef __cplusplus
}
#endif

#endif
