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
