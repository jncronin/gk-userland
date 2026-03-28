#ifndef LINK_H
#define LINK_H

#include <stddef.h>
#include <stdint.h>

#if UINTPTR_MAX == 0xffffffffffffffffu
#include <sys/_elf64.h>
#elif UINTPTR_MAX == 0xffffffff
#error elf32 not supported
#else
#error unsupported UINTPTR_MAX size
#endif

#define ElfW(type)	_ElfW (Elf, __ELF_NATIVE_CLASS, type)
#define _ElfW(e,w,t)	_ElfW_1 (e, w, _##t)
#define _ElfW_1(e,w,t)	e##w##t

#ifdef __cplusplus
extern "C" {
#endif

struct dl_phdr_info
{
    ElfW(Addr) dlpi_addr;
    const char *dlpi_name;
    const ElfW(Phdr) *dlpi_phdr;
    ElfW(Half) dlpi_phnum;
};

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *), void *data);

#ifdef __cplusplus
}
#endif

#endif
