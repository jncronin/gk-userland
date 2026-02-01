#define _RETARGETABLE_LOCKING 1
#include <sys/lock.h>
#include <pthread.h>
#include <cstdint>

/* struct __lock
{
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
}; */

pthread_mutex_t __sfp_recursive_mutex;
pthread_mutex_t __atexit_recursive_mutex;
pthread_mutex_t __at_quick_exit_mutex;
pthread_mutex_t __malloc_recursive_mutex;
pthread_mutex_t __env_recursive_mutex;
pthread_mutex_t __tz_mutex;
pthread_mutex_t __dd_hash_mutex;
pthread_mutex_t __arc4random_mutex;

extern "C"
{

/*
    Fixed locks (i.e. those above) will pass the address of the _ member
    
    Variable locks (created with __retarget_lock_init etc) should do the same therefore

    For creation they pass &_LOCK_T
    For 
*/

void
__retarget_lock_init (_LOCK_T *plock)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    
    pthread_mutex_init(plock, &attr);
}

void
__retarget_lock_init_recursive(_LOCK_T *plock)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    /* We want _LOCK_T to be pthread_mutex_t whereas really it is defined as *__lock */

    pthread_mutex_init(plock, &attr);
}

void
__retarget_lock_close(_LOCK_T *lock)
{
    pthread_mutex_destroy(lock);
}

void
__retarget_lock_close_recursive(_LOCK_T *lock)
{
    pthread_mutex_destroy(lock);
}

void
__retarget_lock_acquire (_LOCK_T *lock)
{
    pthread_mutex_lock(lock);
}

void
__retarget_lock_acquire_recursive (_LOCK_T *lock)
{
    pthread_mutex_lock(lock);
}

int
__retarget_lock_try_acquire(_LOCK_T *lock)
{
    return pthread_mutex_trylock(lock);
}

int
__retarget_lock_try_acquire_recursive(_LOCK_T *lock)
{
    return pthread_mutex_trylock(lock);
}

void
__retarget_lock_release (_LOCK_T *lock)
{
    pthread_mutex_unlock(lock);
}

void
__retarget_lock_release_recursive (_LOCK_T *lock)
{
    pthread_mutex_unlock(lock);
}

void
__gk_newlib_init_locks()
{
    __retarget_lock_init_recursive(&__sfp_recursive_mutex);
    __retarget_lock_init_recursive(&__atexit_recursive_mutex);
    __retarget_lock_init_recursive(&__malloc_recursive_mutex);
    __retarget_lock_init_recursive(&__env_recursive_mutex);
    __retarget_lock_init(&__at_quick_exit_mutex);
    __retarget_lock_init(&__tz_mutex);
    __retarget_lock_init(&__dd_hash_mutex);
    __retarget_lock_init(&__arc4random_mutex);
}

}
