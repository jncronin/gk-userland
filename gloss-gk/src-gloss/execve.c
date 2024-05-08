#include <syscalls.h>
#include <errno.h>
#include <stdarg.h>

#define VA_ARGV_MAX 32

int _execve(char *name,
		char **argv,
		char **env)
{
	struct __syscall_execve_params p;
	p.name = name;
	p.argv = argv;
	p.env = env;
	int ret, _errno;
	__syscall(__syscall_execve, &ret, &p, &_errno);
	if(ret)
	{
		errno = _errno;
	}
	return ret;
}

int execlp(const char *file, const char *arg, ...)
{
	int ret, _errno;
	struct __syscall_execve_params p;
	va_list va;
	size_t nargp;
	const char *argv[VA_ARGV_MAX];
	char *envend = NULL;

	va_start(va, arg);
	while(nargp < VA_ARGV_MAX)
	{
		const char *cargp = va_arg(va, const char *);
		argv[nargp++] = cargp;
		if(cargp == NULL)
		{
			break;
		}
	}
	va_end(va);
	if(nargp >= (VA_ARGV_MAX - 1))
	{
		errno = E2BIG;
		return -1;
	}
	argv[VA_ARGV_MAX - 1] = NULL;

	p.name = (char *)file;
	p.env = &envend;
	p.argv = (char **)argv;

	__syscall(__syscall_execve, &ret, &p, &_errno);
	if(ret)
	{
		errno = _errno;
	}
	return ret;
}