#define _POSIX_READER_WRITER_LOCKS
#define _UNIX98_THREAD_MUTEX_ATTRIBUTES


#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "syscalls.h"
#include "deferred.h"

extern "C" int sem_close(sem_t *sem)
{
    errno = EINVAL;
    return -1;
}

extern "C" int sem_destroy(sem_t *sem)
{
    if(!sem)
    {
        errno = EINVAL;
        return -1;
    }
    return deferred_call(__syscall_sem_destroy, (void *)sem);
}

extern "C" int sem_getvalue(sem_t *sem, int *valout)
{
    if(!sem || !valout)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_sem_getvalue_params p { sem, valout };
    return deferred_call(__syscall_sem_getvalue, &p);
}

extern "C" int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if(!sem)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_sem_init_params p { sem, pshared, value };
    return deferred_call(__syscall_sem_init, &p);
}

extern "C" sem_t *sem_open(const char *name , int oflag, ...)
{
    errno = EINVAL;
    return nullptr;
}

extern "C" int sem_post(sem_t *sem)
{
    if(!sem)
    {
        errno = EINVAL;
        return -1;
    }
    return deferred_call(__syscall_sem_post, (void *)sem);
}

extern "C" int sem_timedwait(sem_t *sem, const timespec *abstime)
{
    if(!sem || !abstime)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = sem, .clock_id = CLOCK_REALTIME, .until = abstime };
    return deferred_call_with_retry(__syscall_sem_trywait, &p, abstime);
}

extern "C" int sem_trywait(sem_t *sem)
{
    if(!sem)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = sem, .clock_id = CLOCK_TRY_ONCE };
    return deferred_call(__syscall_sem_trywait, &p);
}

extern "C" int sem_unlink(const char *name)
{
    errno = EINVAL;
    return -1;
}

extern "C" int sem_wait(sem_t *sem)
{
    if(!sem)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = sem, .clock_id = CLOCK_WAIT_FOREVER };
    return deferred_call_with_retry(__syscall_sem_trywait, &p);
}
