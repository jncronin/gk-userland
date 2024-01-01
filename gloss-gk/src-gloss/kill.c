#include <gk_syscalls.h>

int _kill(int pid, int sig)
{
	return __gk_syscalls->_kill(pid, sig);
}
