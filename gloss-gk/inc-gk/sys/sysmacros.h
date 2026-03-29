#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

dev_t makedev(unsigned int maj, unsigned int min);

unsigned int major(dev_t dev);
unsigned int minor(dev_t dev);

#ifdef __cplusplus
}
#endif

#endif
