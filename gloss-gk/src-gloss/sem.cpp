#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

extern "C" int sem_close(sem_t *sem)
{
    errno = EINVAL;
    return -1;
}

extern "C" int sem_destroy(sem_t *sem)
{
    errno = EINVAL;
    return -1;
}

extern "C" int sem_getvalue(sem_t *sem, int *valout)
{
    errno = EINVAL;
    return -1;
}

extern "C" int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    errno = EINVAL;
    return -1;
}

extern "C" sem_t *sem_open(const char *name , int oflag, ...)
{
    errno = EINVAL;
    return nullptr;
}

extern "C" int sem_post(sem_t *sem)
{
    errno = EINVAL;
    return -1;
}

extern "C" int sem_timedwait(sem_t *sem, const timespec *abstime)
{
    errno = EINVAL;
    return -1;
}

extern "C" int sem_trywait(sem_t *sem)
{
    errno = EINVAL;
    return -1;
}

extern "C" int sem_unlink(const char *name)
{
    errno = EINVAL;
    return -1;
}

extern "C" int sem_wait(sem_t *sem)
{
    errno = EINVAL;
    return -1;
}
