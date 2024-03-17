#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "syscalls.h"
#include "deferred.h"

int pthread_attr_init(pthread_attr_t *attr)
{
    memset(attr, 0, sizeof(pthread_attr_t));
    attr->is_initialized = 1;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    memset(attr, 0, sizeof(pthread_attr_t));
    return 0;
}

int pthread_create(pthread_t *thread,
    const pthread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg)
{
    __syscall_pthread_create_params p
    { thread, attr, start_routine, arg };
    return deferred_call(__syscall_pthread_create, &p);
}
