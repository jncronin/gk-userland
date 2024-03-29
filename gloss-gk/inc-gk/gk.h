#ifndef GK_H
#define GK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "_gk_gpu.h"

#include <stdlib.h>

ssize_t GK_GPUEnqueueMessages(const struct gpu_message *msgs, size_t nmsg);



#ifdef __cplusplus
}
#endif

#endif
