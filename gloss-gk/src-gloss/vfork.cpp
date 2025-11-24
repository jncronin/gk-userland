#include <unistd.h>

extern "C" pid_t vfork()
{
    return (pid_t)-1;
}
