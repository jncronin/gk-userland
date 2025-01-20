#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

static struct passwd retval;
static struct group grp;
static const char *grp_0_members[] = { "root", 0 };
static const char *grp_1_members[] = { "user", 0 };

struct passwd *getpwuid(uid_t uid)
{
    switch(uid)
    {
        case 0:
            retval.pw_name = "root";
            retval.pw_dir = "/root";
            retval.pw_passwd = "00root       ";
            retval.pw_uid = 0;
            retval.pw_gid = 0;
            retval.pw_gecos = "";
            retval.pw_shell = "/bin/sh";
            return &retval;

        case 1:
            retval.pw_name = "user";
            retval.pw_dir = "/home/user";
            retval.pw_passwd = "00user       ";
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

uid_t geteuid()
{
    return 1;
}

int issetugid()
{
    return 0;
}

struct group *getgrgid(gid_t gid)
{
    switch(gid)
    {
        case 0:
            grp.gr_name = "root";
            grp.gr_passwd = 0;
            grp.gr_gid = 0;
            grp.gr_mem = (char **)grp_0_members;
            return &grp;

        case 1:
            grp.gr_name = "user";
            grp.gr_passwd = 0;
            grp.gr_gid = 1;
            grp.gr_mem = (char **)grp_1_members;
            return &grp;

        default:
            return NULL;
    }
}

struct group *getgrnam(const char *name)
{
    if(!strcmp("root", name))
        return getgrgid(0);
    else if(!strcmp("user", name))
        return getgrgid(1);
    else
        return NULL;
}

void setpwent(void)
{

}

void endpwent(void)
{

}
