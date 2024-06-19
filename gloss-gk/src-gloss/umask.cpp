#include <sys/stat.h>
#include <errno.h>

extern "C" mode_t umask(mode_t mask)
{
    // stub
    return 0644;    // octal
}
