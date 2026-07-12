#ifndef _DLFCN_H
#define _DLFCN_H

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_LAZY           1
#define RTLD_NOW            2
#define RTLD_GLOBAL         4
#define RTLD_LOCAL          8

int dlclose(void *);
char *dlerror(void);
void *dlopen(const char *, int);
void *dlsym(void *, const char *);
int dlinfo(void *handle, int request, void *info);

#define RTLD_DI_LMID        1
#define RTLD_DI_LINKMAP     2
#define RTLD_DI_ORIGIN      3
#define RTLD_DI_SERINFO     4
#define RTLD_DI_SERINFOSIZE 5
#define RTLD_DI_TLS_MODID   6
#define RTLD_DI_TLS_DATA    7
#define RTLD_DI_PHDR        8

typedef struct
{
    const char *dli_fname;
    void *dli_fbase;
    const char *dli_sname;
    void *dli_sbase;
} Dl_info_t;

typedef Dl_info_t Dl_info;

int dladdr(const void *addr, Dl_info_t *dlip);

#ifdef __cplusplus
}
#endif

#endif
