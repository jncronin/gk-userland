#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#ifdef __cplusplus
extern "C" {
#endif

struct utsname
{
    char        sysname[32];
    char        nodename[32];
    char        release[32];
    char        version[32];
    char        machine[32];
};

int uname(struct utsname *);

#ifdef __cplusplus
}
#endif

#endif
