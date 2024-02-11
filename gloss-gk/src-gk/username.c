#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

static struct passwd retval;

struct passwd *getpwuid(uid_t uid)
{
    switch(uid)
    {
        case 0:
            retval.pw_name = "root";
            retval.pw_dir = "/root";
            retval.pw_passwd = "";
            retval.pw_uid = 0;
            retval.pw_gid = 0;
            retval.pw_gecos = "";
            retval.pw_shell = "/bin/sh";
            return &retval;

        case 1:
            retval.pw_name = "user";
            retval.pw_dir = "/home/user";
            retval.pw_passwd = "";
            retval.pw_uid = 1;
            retval.pw_gid = 1;
            retval.pw_gecos = "";
            retval.pw_shell = "/bin/sh";
            return &retval;

        default:
            return NULL;
    }
}

struct passwd *getpwnam(const char *name)
{
    if(!name)
    {
        errno = EINVAL;
        return NULL;
    }

    if(strcmp("root", name) == 0)
    {
        return getpwuid(0);
    }

    if(strcmp("user", name) == 0)
    {
        return getpwuid(1);
    }

    return NULL;
}

char *getlogin()
{
    return "user";
}

uid_t getuid()
{
    return 1;
}

int issetugid()
{
    return 0;
}
