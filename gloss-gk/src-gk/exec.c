#include <unistd.h>

int execvp(const char *file, char *const argv[])
{
    char *envp[1];
    envp[0] = NULL;
    return execve(file, argv, envp);
}
