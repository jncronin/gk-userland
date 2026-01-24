#include <syscalls.h>
#include <deferred.h>
#include <pthread.h>
#include <stdlib.h>

extern "C" void _exit(int rc)
{
	__syscall(__syscall_exit, (void *)(uintptr_t)rc, NULL, NULL);
	while(true);
}
