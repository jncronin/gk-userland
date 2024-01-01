#include <pthread.h>
#include <stdlib.h>
#include <string.h>

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
