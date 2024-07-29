#ifndef MALLOC_PROVIDED
/*
FUNCTION
<<__malloc_lock>>, <<__malloc_unlock>>---lock malloc pool

INDEX
	__malloc_lock
INDEX
	__malloc_unlock

SYNOPSIS
	#include <malloc.h>
	void __malloc_lock (struct _reent *<[reent]>);
	void __malloc_unlock (struct _reent *<[reent]>);

DESCRIPTION
The <<malloc>> family of routines call these functions when they need to lock
the memory pool.  The version of these routines supplied in the library use
the lock API defined in sys/lock.h.  If multiple threads of execution can
call <<malloc>>, or if <<malloc>> can be called reentrantly, then you need to
define your own versions of these functions in order to safely lock the
memory pool during a call.  If you do not, the memory pool may become
corrupted.

A call to <<malloc>> may call <<__malloc_lock>> recursively; that is,
the sequence of calls may go <<__malloc_lock>>, <<__malloc_lock>>,
<<__malloc_unlock>>, <<__malloc_unlock>>.  Any implementation of these
routines must be careful to avoid causing a thread to wait for a lock
that it already holds.
*/

#ifdef __GAMEKID__
#define _POSIX_THREADS 1
#define _UNIX98_THREAD_MUTEX_ATTRIBUTES 1
#include <pthread.h>
#endif

#include <malloc.h>
#include <sys/lock.h>


#ifdef __GAMEKID__
static pthread_mutex_t __malloc_mutex;
static int __lock_init = 0;
void __malloc_lock(ptr)
	struct _reent *ptr;
{
	if(!__lock_init)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		
		pthread_mutex_init(&__malloc_mutex, &attr);
		__lock_init = 1;
	}

	pthread_mutex_lock(&__malloc_mutex);
}

void __malloc_unlock(ptr)
	struct _reent *ptr;
{
	if(__lock_init)
	{
		pthread_mutex_unlock(&__malloc_mutex);
	}
}

#else
#ifndef __SINGLE_THREAD__
__LOCK_INIT_RECURSIVE(static, __malloc_recursive_mutex);
#endif

void
__malloc_lock (ptr)
     struct _reent *ptr;
{
#ifndef __SINGLE_THREAD__
  __lock_acquire_recursive (__malloc_recursive_mutex);
#endif
}

void
__malloc_unlock (ptr)
     struct _reent *ptr;
{
#ifndef __SINGLE_THREAD__
  __lock_release_recursive (__malloc_recursive_mutex);
#endif
}

#endif

#endif
