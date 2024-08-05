#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
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

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
    if(!attr)
    {
        return EINVAL;
    }
    if(detachstate != PTHREAD_CREATE_DETACHED &&
        detachstate != PTHREAD_CREATE_JOINABLE)
    {
        return EINVAL;
    }
    attr->detachstate = detachstate;
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    if(!attr)
    {
        return EINVAL;
    }
    if(stacksize < 64)
    {
        return EINVAL;
    }
    attr->stacksize = stacksize;
    return 0;
}

extern "C" int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
    return 0;
}

pthread_t pthread_self()
{
    auto taddr = __syscall_GetThreadHandle();
    return (pthread_t)((uint32_t)(uintptr_t)taddr);
}

extern "C" int pthread_setname_np(pthread_t thread, const char *name)
{
    __syscall_pthread_setname_np_params p { thread, name };
    return deferred_call(__syscall_pthread_setname_np, &p);
}

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset)
{
    __syscall_pthread_sigmask_params p { how, set, oldset };
    return deferred_call(__syscall_pthread_sigmask, &p);
}

extern "C" int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    return pthread_sigmask(how, set, oldset);
}

int pthread_setcanceltype(int type, int *oldtype)
{
    if(oldtype)
        *oldtype = PTHREAD_CANCEL_DEFERRED;
    return 0;
}

int pthread_setcancelstate(int state, int *oldstate)
{
    if(oldstate)
        *oldstate = PTHREAD_CANCEL_ENABLE;
    return 0;
}

extern "C" int pthread_getschedparam(pthread_t t, int *policy, sched_param *param)
{
    if(policy)
        *policy = SCHED_RR;
    int sched_priority = deferred_call(__syscall_get_thread_priority, (void *)((uint32_t)t));
    if(sched_priority > 0)
    {
        if(param)
            param->sched_priority = sched_priority;
        return 0;
    }
    return ESRCH;
}

extern "C" int pthread_setschedparam(pthread_t t, int policy, const sched_param *param)
{
    if(!param)
        return EINVAL;
    if(param->sched_priority < 0)
        return EINVAL;
    if(param->sched_priority > 9)
        return EINVAL;
    __syscall_set_thread_priority_params p { (void *)((uint32_t)t), param->sched_priority };
    int ret = deferred_call(__syscall_set_thread_priority, &p);
    if(ret == 0)
        return 0;
    return EPERM;
}

extern "C" int sched_get_priority_min(int policy)
{
    return 0;
}

extern "C" int sched_get_priority_max(int policy)
{
    return 9;
}

int pthread_join(pthread_t thread, void **retval)
{
    __syscall_pthread_join_params p { (void *)((uint32_t)thread), retval };
    int ret = deferred_call(__syscall_pthread_join, &p);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_detach(pthread_t thread)
{
    int ret = deferred_call(__syscall_pthread_detach, (void *)thread);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_cancel(pthread_t thread)
{
    int ret = deferred_call(__syscall_pthread_cancel, (void *)thread);
    if(ret == 0)
        return 0;
    return errno;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if(!attr)
        return EINVAL;
    attr->recursive = 0;
    attr->is_initialized = 1;
    attr->type = PTHREAD_MUTEX_DEFAULT;
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    if(!attr)
        return EINVAL;
    attr->recursive = 0;
    attr->is_initialized = 0;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if(!attr)
        return EINVAL;
    attr->type = type;
    attr->recursive = (type == PTHREAD_MUTEX_RECURSIVE) ? 1 : 0;
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    if(!attr || !type)
        return EINVAL;
    *type = attr->type;
    return 0;
}

extern "C" int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
    attr->is_initialized = 1;
    return 0;
}

extern "C" int pthread_rwlock_init(pthread_rwlock_t *lock,
    const pthread_rwlockattr_t *attr)
{
    pthread_rwlockattr_t defattr;
    if(!attr)
    {
        pthread_rwlockattr_init(&defattr);
        attr = &defattr;
    }
    if(!attr->is_initialized)
    {
        return EINVAL;
    }
    __syscall_pthread_rwlock_init_params p { lock, attr };
    int ret = deferred_call(__syscall_pthread_rwlock_init, &p);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_rwlock_destroy(pthread_rwlock_t *lock)
{
    int ret = deferred_call(__syscall_pthread_rwlock_destroy, lock);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_rwlock_rdlock(pthread_rwlock_t *lock)
{
    if(!lock)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = lock, .clock_id = CLOCK_WAIT_FOREVER };
    int ret = deferred_call_with_retry(__syscall_pthread_rwlock_tryrdlock, &p);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_rwlock_timedrdlock(pthread_rwlock_t *lock, const timespec *abstime)
{
    if(!lock || !abstime)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = lock, .clock_id = CLOCK_REALTIME, .until = abstime };
    int ret = deferred_call_with_retry(__syscall_pthread_rwlock_tryrdlock, &p, abstime);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_rwlock_tryrdlock(pthread_rwlock_t *lock)
{
    if(!lock)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = lock, .clock_id = CLOCK_TRY_ONCE };
    int ret = deferred_call(__syscall_pthread_rwlock_tryrdlock, &p);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_rwlock_wrlock(pthread_rwlock_t *lock)
{
    if(!lock)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = lock, .clock_id = CLOCK_WAIT_FOREVER };
    int ret = deferred_call_with_retry(__syscall_pthread_rwlock_trywrlock, &p);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_rwlock_timedwrlock(pthread_rwlock_t *lock, const timespec *abstime)
{
    if(!lock || !abstime)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = lock, .clock_id = CLOCK_REALTIME, .until = abstime };
    int ret = deferred_call_with_retry(__syscall_pthread_rwlock_trywrlock, &p, abstime);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_rwlock_trywrlock(pthread_rwlock_t *lock)
{
    if(!lock)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = lock, .clock_id = CLOCK_TRY_ONCE };
    int ret = deferred_call(__syscall_pthread_rwlock_trywrlock, &p);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_rwlock_unlock(pthread_rwlock_t *lock)
{
    int ret = deferred_call(__syscall_pthread_rwlock_unlock, lock);
    if(ret == 0)
        return 0;
    return errno;
}

int pthread_mutex_init(pthread_mutex_t *mutex, 
    const pthread_mutexattr_t *attr)
{
    pthread_mutexattr_t defattr;
    if(!attr)
    {
        pthread_mutexattr_init(&defattr);
        attr = &defattr;
    }
    if(!attr->is_initialized)
    {
        return EINVAL;
    }
    __syscall_pthread_mutex_init_params p { mutex, attr };
    int ret = deferred_call(__syscall_pthread_mutex_init, &p);
    if(ret == 0)
        return 0;
    return errno;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    int ret = deferred_call(__syscall_pthread_mutex_destroy, mutex);
    if(ret == 0)
        return 0;
    return errno;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    if(!mutex)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = mutex, .clock_id = CLOCK_WAIT_FOREVER };
    int ret = deferred_call_with_retry(__syscall_pthread_mutex_trylock, &p);
    if(ret == 0)
        return 0;
    return errno;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    if(!mutex)
    {
        errno = EINVAL;
        return -1;
    }
    __syscall_trywait_params p { .sync = mutex, .clock_id = CLOCK_TRY_ONCE };
    int ret = deferred_call(__syscall_pthread_mutex_trylock, &p);
    if(ret == 0)
        return 0;
    if(ret == -3)
        return EBUSY;

    return errno;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    int ret = deferred_call(__syscall_pthread_mutex_unlock, mutex);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_key_create(pthread_key_t *key,
    void (*destructor)(void *))
{
    __syscall_pthread_key_create_params p { key, destructor };
    int ret = deferred_call(__syscall_pthread_key_create, &p);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_key_delete(pthread_key_t key)
{
    int ret = deferred_call(__syscall_pthread_key_delete, key);
    if(ret == 0)
        return 0;
    return errno;
}

void *pthread_getspecific(pthread_key_t key)
{
    void *retp;
    __syscall_pthread_getspecific_params p { key, &retp };
    int ret = deferred_call(__syscall_pthread_getspecific, &p);
    if(ret == 0)
        return retp;
    return nullptr;
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
    __syscall_pthread_setspecific_params p { key, value };
    int ret = deferred_call(__syscall_pthread_setspecific, &p);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_condattr_init(pthread_condattr_t *attr)
{
    if(!attr)
    {
        return EINVAL;
    }
    attr->clock = 0;
    attr->is_initialized = 1;
    return 0;
}

extern "C" int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock)
{
    if(!attr)
    {
        return EINVAL;
    }
    attr->clock = clock;
    return 0;
}

extern "C" int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock)
{
    if(!attr || !clock)
    {
        return EINVAL;
    }
    *clock = attr->clock;
    return 0;
}

extern "C" int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    if(!attr)
    {
        return EINVAL;
    }
    attr->is_initialized = 0;
    return 0;
}

int pthread_cond_init(pthread_cond_t *cond,
    const pthread_condattr_t *attr)
{
    pthread_condattr_t defattr;
    if(!attr)
    {
        pthread_condattr_init(&defattr);
        attr = &defattr;
    }
    if(!attr->is_initialized)
    {
        return EINVAL;
    }
    __syscall_pthread_cond_init_params p { cond, attr };
    int ret = deferred_call(__syscall_pthread_cond_init, &p);
    if(ret == 0)
        return 0;
    return errno;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    int ret = deferred_call(__syscall_pthread_cond_destroy, cond);
    if(ret == 0)
        return 0;
    return errno;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    __syscall_pthread_cond_timedwait_params p { cond, mutex, nullptr, nullptr };
    int ret = deferred_call(__syscall_pthread_cond_timedwait, &p);
    if(ret == 0)
        return 0;
    return errno;
}

int pthread_cond_timedwait(pthread_cond_t *cond, 
    pthread_mutex_t *mutex, const struct timespec *abstime)
{
    int signalled = 0;
    __syscall_pthread_cond_timedwait_params p { cond, mutex, abstime, &signalled };
    int ret = deferred_call(__syscall_pthread_cond_timedwait, &p);
    if(ret == 0)
    {
        if(signalled)
            return 0;
        else
            return ETIMEDOUT;
    }
    return errno;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    int ret = deferred_call(__syscall_pthread_cond_signal, cond);
    if(ret == 0)
        return 0;
    return errno;
}

extern "C" int pthread_cond_broadcast(pthread_cond_t *cond)
{
    return pthread_cond_signal(cond);
}

void pthread_exit(void *retval)
{
    deferred_call(__syscall_pthread_exit, &retval);
    while(true);
}
