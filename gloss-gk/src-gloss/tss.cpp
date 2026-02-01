#include "threads.h"
#include "pthread.h"
#include <cstdint>

extern "C"
{

int
tss_create(tss_t *key, tss_dtor_t dtor)
{
	if (pthread_key_create((pthread_key_t *)key, dtor) != 0)
		return (thrd_error);
	return (thrd_success);
}

void
tss_delete(tss_t key)
{

	(void)pthread_key_delete((pthread_key_t)(uintptr_t)key);
}

void *
tss_get(tss_t key)
{

	return (pthread_getspecific((pthread_key_t)(uintptr_t)key));
}

int
tss_set(tss_t key, void *val)
{

	if (pthread_setspecific((pthread_key_t)(uintptr_t)key, val) != 0)
		return (thrd_error);
	return (thrd_success);
}

}
