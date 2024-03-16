#include <syscalls.h>
#include "deferred.h"
#include <errno.h>

extern "C" int _close(int file)
{
	auto ret = deferred_call(__syscall_close1, file);
	if(ret != 0)
		return ret;
	return deferred_call(__syscall_close2, file);
}
