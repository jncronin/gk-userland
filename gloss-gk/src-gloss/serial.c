/* Stub serial functions.  We only support one serial device which has fixed features. */

#include <termios.h>

int tcsetattr(int fildes, int optional_actions, const struct termios *termios_p)
{
    return 0;
}

int tcgetattr(int fildes, struct termios *termios_p)
{
    return 0;
}

int cfsetospeed(struct termios *termios_p, speed_t speed)
{
    return 0;
}

int cfsetispeed(struct termios *termios_p, speed_t speed)
{
    return 0;
}





