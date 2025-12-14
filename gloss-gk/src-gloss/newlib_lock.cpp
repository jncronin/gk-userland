#define _RETARGETABLE_LOCKING 1
#include <sys/lock.h>
#include <pthread.h>

struct __lock
{
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
};

struct __lock __lock___sfp_recursive_mutex;
struct __lock __lock___atexit_recursive_mutex;
struct __lock __lock___at_quick_exit_mutex;
struct __lock __lock___malloc_recursive_mutex;
struct __lock __lock___env_recursive_mutex;
struct __lock __lock___tz_mutex;
struct __lock __lock___dd_hash_mutex;
struct __lock __lock___arc4random_mutex;

extern "C"
{

void
__retarget_lock_init (_LOCK_T *lock)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    
    pthread_mutex_init(&(*lock)->m, &attr);
}

void
__retarget_lock_init_recursive(_LOCK_T *lock)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    
    pthread_mutex_init(&(*lock)->m, &attr);
}

void
__retarget_lock_close(_LOCK_T lock)
{
    pthread_mutex_destroy(&lock->m);
}

void
__retarget_lock_close_recursive(_LOCK_T lock)
{
    pthread_mutex_destroy(&lock->m);
}

void
__retarget_lock_acquire (_LOCK_T lock)
{
    pthread_mutex_lock(&lock->m);
}

void
__retarget_lock_acquire_recursive (_LOCK_T lock)
{
    pthread_mutex_lock(&lock->m);
}

int
__retarget_lock_try_acquire(_LOCK_T lock)
{
    return pthread_mutex_trylock(&lock->m);
}

int
__retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    return pthread_mutex_trylock(&lock->m);
}

void
__retarget_lock_release (_LOCK_T lock)
{
    pthread_mutex_unlock(&lock->m);
}

void
__retarget_lock_release_recursive (_LOCK_T lock)
{
    pthread_mutex_unlock(&lock->m);
}

}
