#include "dlfcn.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "link.h"
#include "deferred.h"
#include "syscalls.h"
#include "limits.h"
#include <string.h>

#if __GAMEKID__ >= 4
#include <stdint.h>
static void * const __dl_main_exec = (void *)-1;

struct _dlinfo
{
    int fd;
    char *name;
    ElfW(Ehdr) *eh;
    void *baseaddr;
};

static int getndl();
static _dlinfo getdl(int dl_id);
static _dlinfo getdl(void *handle);
static void freedl(struct _dlinfo &dl);

void *dlopen(const char *file, int mode)
{
    if(!file)
    {
        // open main exec
        return __dl_main_exec;
    }
    else
    {
#if 1
        return __dl_main_exec;
#else
        // just try and open
        auto oret = open(file, O_RDONLY);
        if(oret >= 0)
        {
            // TODO: load into address space and register with getdl()
            return (void *)(intptr_t)oret;
        }
        else
        {
            return nullptr;
        }
#endif
    }
}

int dlclose(void *handle)
{
    if(handle == __dl_main_exec)
    {
        return 0;
    }

    close((int)(intptr_t)handle);
    return 0;
}

static int getndl()
{
    int dummy = 0;
    return deferred_call(__syscall_getndl, &dummy);
}

static _dlinfo getdl(int dl_id)
{
    size_t fname_len = 256;
    char *fname = (char *)malloc(fname_len);
    _dlinfo ret;

//    fprintf(stderr, "getdl(%d) begin\n", dl_id);

    while(fname && fname_len < PATH_MAX)
    {
        __syscall_getdl_params p
        {
            .dl_id = dl_id,
            .fd = &ret.fd,
            .name = fname,
            .namelen = &fname_len,
            .img = (void **)&ret.eh,
            .baseaddr = &ret.baseaddr
        };
        auto sret = deferred_call(__syscall_getdl, &p);

        //fprintf(stderr, "getdl(%d): sret: %d, fname_len: %u\n", dl_id, sret, fname_len);

        if(sret == -1)
        {
            if(fname_len == 0)
            {
                // error - doesn't exist
                break;
            }
            else if(fname_len < PATH_MAX)
            {
                fname = (char *)realloc(fname, fname_len);
            }
        }
        else
        {
            // success
            ret.name = fname;
            return ret;
        }
    }

    if(fname)
    {
        free(fname);
    }

    ret.fd = -1;
    ret.name = nullptr;
    ret.eh = nullptr;
    ret.baseaddr = nullptr;
    return ret;
}

static _dlinfo getdl(void *handle)
{
    auto fd = (int)(intptr_t)handle;

    if(fd == -1)
    {
        return getdl(0);
    }

    auto ndls = getndl();
    for(int i = 0; i < ndls; i++)
    {
        auto cdl = getdl(i);
        if(cdl.fd == fd)
        {
            return cdl;
        }
    }

    _dlinfo ret;
    ret.fd = -1;
    ret.name = nullptr;
    ret.eh = nullptr;
    ret.baseaddr = nullptr;
    return ret;
}

static void freedl(struct _dlinfo &dl)
{
    if(dl.fd >= 0 && dl.name)
    {
        free(dl.name);
    }
}

static const ElfW(Shdr) *get_symtab(const _dlinfo &dl)
{
    if(dl.eh == nullptr)
    {
        return nullptr;
    }

    const auto ehaddr = (uintptr_t)dl.eh;
    const auto shdr_addr = ehaddr + dl.eh->e_shoff;
    const auto shstr = (const ElfW(Shdr *))(ehaddr + dl.eh->e_shoff +
        dl.eh->e_shstrndx * dl.eh->e_shentsize);
    const char *shstrs = (const char *)(ehaddr +
        shstr->sh_offset);
    for(auto i = 0U; i < dl.eh->e_shnum; i++)
    {
        const auto shdr = (const ElfW(Shdr *))(ehaddr + dl.eh->e_shoff +
            i * dl.eh->e_shentsize);
        const auto sname = &shstrs[shdr->sh_name];
        if(!strcmp(".symtab", sname))
        {
            return shdr;
        }
    }

    return nullptr;
}

int dladdr(const void *addr, Dl_info *info)
{
    const auto testaddr = (uintptr_t)addr;

    for(auto j = 0; j < getndl(); j++)
    {
        auto dl = getdl(j);
        if(dl.eh)
        {
            const auto ehaddr = (uintptr_t)dl.eh;
            const auto symtab = get_symtab(dl);
            if(symtab)
            {
                const auto symstrhdr = (const ElfW(Shdr) *)
                    (ehaddr + dl.eh->e_shoff +
                    symtab->sh_link * dl.eh->e_shentsize);
                const char *symstrs = (const char *)(ehaddr +
                    symstrhdr->sh_offset);
                const auto nsyms = symtab->sh_size /
                    symtab->sh_entsize;
                
                for(auto i = symtab->sh_info; i < nsyms; i++)
                {
                    const auto csym = (const ElfW(Sym) *)
                        (ehaddr + symtab->sh_offset +
                        i * symtab->sh_entsize);
                    const char *csymname = &symstrs[csym->st_name];

                    const auto sym_start = (uintptr_t)dl.baseaddr +
                        (uintptr_t)csym->st_value;
                    const auto sym_end = sym_start + csym->st_size;
                    
                    if(testaddr >= sym_start && testaddr <= sym_end)
                    {
                        // found
                        info->dli_fname = (const char *)malloc(strlen(dl.name) + 1);
                        info->dli_sname = (const char *)malloc(strlen(csymname) + 1);
                        if(!info->dli_fname || !info->dli_sname)
                        {
                            if(info->dli_fname)
                                free((char *)info->dli_fname);
                            if(info->dli_sname)
                                free((char *)info->dli_sname);
                            freedl(dl);
                            errno = ENOMEM;
                            return -1;
                        }
                        strcpy((char *)info->dli_fname, dl.name);
                        strcpy((char *)info->dli_sname, csymname);

                        info->dli_fbase = dl.baseaddr;
                        info->dli_sbase = (void *)sym_start;

                        freedl(dl);

                        return 1; // on success return non-zero!
                    }
                }
            }

            freedl(dl);
        }
    }

    fprintf(stderr, "dladdr failed for %p\n", addr);

    return 0;   // fail return 0
}

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *), void *data)
{
    for(auto j = 0; j < getndl(); j++)
    {
        auto dl = getdl(j);
        if(dl.eh)
        {
            const auto ehaddr = (uintptr_t)dl.eh;

            dl_phdr_info info;
            info.dlpi_addr = (ElfW(Addr))dl.baseaddr;
            info.dlpi_name = dl.name;
            info.dlpi_phdr = (const ElfW(Phdr) *)malloc(
                dl.eh->e_phnum * sizeof(ElfW(Phdr)));
            if(!info.dlpi_phdr)
            {
                freedl(dl);
                errno = ENOMEM;
                return -1;
            }

            // handle the case where e_phentsize != sizeof(ElfW(Phdr))
            for(auto i = 0u; i < dl.eh->e_phnum; i++)
            {
                memcpy((void *)&info.dlpi_phdr[i],
                    (void *)(ehaddr + dl.eh->e_phoff +
                    i * dl.eh->e_phentsize),
                    sizeof(ElfW(Phdr)));
            }

            info.dlpi_phnum = dl.eh->e_phnum;

            auto ret = callback(&info, sizeof(dl_phdr_info), data);

            free((void *)info.dlpi_phdr);
            freedl(dl);

            if(ret)
                return ret;
        }
    }

    return 0;
}

void *dlsym(void *handle, const char *name)
{
    //fprintf(stderr, "dlsym(%s) begin\n", name);
    auto dl = getdl(handle);
    //fprintf(stderr, "dlsym(%s): dl: { fd: %d, name: %s, baseaddr: %p, eh: %p }\n",
    //    name, dl.fd, dl.name, dl.baseaddr, dl.eh);
    if(dl.eh)
    {
        //fprintf(stderr, "dlsym(%s) have dl: %s\n", name, dl.name);
        const auto ehaddr = (uintptr_t)dl.eh;
        const auto symtab = get_symtab(dl);
        if(symtab)
        {
            const auto symstrhdr = (const ElfW(Shdr) *)
                (ehaddr + dl.eh->e_shoff +
                symtab->sh_link * dl.eh->e_shentsize);
            const char *symstrs = (const char *)(ehaddr +
                symstrhdr->sh_offset);
            const auto nsyms = symtab->sh_size /
                symtab->sh_entsize;
            //fprintf(stderr, "dlsym(%s) have symtab: %u syms\n", name, nsyms);
            for(auto i = symtab->sh_info; i < nsyms; i++)
            {
                const auto csym = (const ElfW(Sym) *)
                    (ehaddr + symtab->sh_offset +
                    i * symtab->sh_entsize);
                const char *csymname = &symstrs[csym->st_name];

                //fprintf(stderr, "dlsym(%s) sym %u: %s\n", name, i, csymname);

                if(strcmp(name, csymname) == 0)
                {
                    // found
                    freedl(dl);
                    return (void *)((uintptr_t)dl.baseaddr +
                        (uintptr_t)csym->st_value);
                }
            }
        }
        freedl(dl);
    }
    fprintf(stderr, "dlsym: %s not found within %p\n", name, handle);
    return nullptr;
}

static char _dlerror[] = "dlfcn.h functions are not supported";
char *dlerror()
{
    return _dlerror;
}

#else
// we don't support dl*()

void *dlopen(const char *file, int mode)
{
    fprintf(stderr, "dlopen (%s) called, failing\n", file);
    return nullptr;
}

int dlclose(void *handle)
{
    return -1;
}

void *dlsym(void *handle, const char *name)
{
    return nullptr;
}

static char _dlerror[] = "dlfcn.h functions are not supported";
char *dlerror()
{
    return _dlerror;
}
#endif
