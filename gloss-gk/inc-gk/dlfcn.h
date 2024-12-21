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

#ifdef __cplusplus
}
#endif

#endif
