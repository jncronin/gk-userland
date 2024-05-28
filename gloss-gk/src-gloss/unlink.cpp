#include <syscalls.h>
#include <errno.h>
#include "deferred.h"

extern "C" int _unlink(char *name)
{
	return deferred_call(__syscall_unlink, name);
}

