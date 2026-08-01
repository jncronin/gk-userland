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
#include <sys/mman.h>

#define __GAMEKID__ 4

#if __GAMEKID__ >= 4
#include <stdint.h>
static void * const __dl_main_exec = (void *)-1;

struct _dlinfo
{
    int fd;
    char *name;
    ElfW(Ehdr) *eh;
    void *baseaddr;
    int id;
    bool global;
    const ElfW(Phdr) *_p_dyn = nullptr;
    uintptr_t symtab = 0;
    uintptr_t hash = 0;
    uintptr_t syment = 0;
    uintptr_t nsyms = 0;
    uintptr_t strtab = 0;

    uintptr_t get_sym(const char *symname);
    const ElfW(Phdr) *pdyn();
};

static int getndl();
static _dlinfo getdl(int dl_id);
static _dlinfo getdl(void *handle);
static void freedl(struct _dlinfo &dl);
static int dlfcn_loadimage(struct _dlinfo dl);

class ImageList
{
    protected:
        _dlinfo *dli = nullptr;
        size_t ndli = 0;

    public:
        size_t size() const { return ndli; }
        _dlinfo *get(size_t idx)
        {
            if(idx >= ndli)
                return nullptr;
            return &dli[idx];
        }

        ImageList() = delete;

        ImageList(const ImageList &other)
        {
            ndli = other.ndli;
            dli = (_dlinfo *)malloc(ndli * sizeof(_dlinfo));
            if(!dli)
            {
                fprintf(stderr, "dlopen(): ImageList: OOM\n");
                ndli = 0;
                return;
            }

            memcpy(dli, other.dli, ndli * sizeof(_dlinfo));
        }

        ImageList(ImageList &&other)
        {
            dli = other.dli;
            ndli = other.ndli;

            other.dli = nullptr;
            other.ndli = 0;
        }

        ImageList(int cur_id, bool symbolic = false)
        {
            ndli = getndl();
            dli = (_dlinfo *)malloc(ndli * sizeof(_dlinfo));
            if(!dli)
            {
                fprintf(stderr, "dlopen(): ImageList: OOM\n");
                ndli = 0;
                return;
            }

            if(symbolic)
            {
                dli[0] = getdl(cur_id);

                size_t oidx = 1;
                for(size_t idx = 0; idx < ndli; idx++)
                {
                    auto iidx = ndli - idx - 1;
                    if(iidx == cur_id)
                        continue;
                    dli[oidx++] = getdl(iidx);
                }
            }
            else
            {
                for(size_t idx = 0; idx < ndli; idx++)
                {
                    auto iidx = ndli - idx - 1;
                    dli[idx] = getdl(iidx);
                }
            }
        }

        ~ImageList()
        {
            if(dli)
            {
                for(size_t idx = 0; idx < ndli; idx++)
                {
                    freedl(dli[idx]);
                }
                free(dli);
            }
        }

        uintptr_t getsym(const char *symname, int cur_id) const
        {
            for(size_t idx = 0; idx < ndli; idx++)
            {
                if(!dli[idx].global && idx != cur_id)
                    continue;
                auto csym = dli[idx].get_sym(symname);
                if(csym)
                    return (uintptr_t)dli[idx].baseaddr + csym;
            }
            return 0;
        }
};

void *dlopen(const char *file, int mode)
{
    if(!file)
    {
        // open main exec
        return __dl_main_exec;
    }
    else
    {
        // just try and open
        auto oret = open(file, O_RDONLY);
        if(oret >= 0)
        {
            __syscall_loadimage_params p;
            p.fd = oret;
            p.global = (((mode & RTLD_GLOBAL) != 0) && ((mode & RTLD_LOCAL) == 0)) ? 1 : 0;
            auto liret = deferred_call(__syscall_loadimage, &p);
            if(liret != 0)
            {
                fprintf(stderr, "dlopen: loadimage failed: %d\n", errno);
                close(oret);
                return nullptr;
            }

            // load dependencies, resolve relocations, run init() etc
            auto h = (void *)(intptr_t)oret;
            auto dl = getdl(h);
            if(dlfcn_loadimage(dl) != 0)
            {
                fprintf(stderr, "dlopen: dlfcn_loadimage failed: %d\n", errno);
                close(oret);
                freedl(dl);
                return nullptr;
            }

            freedl(dl);
            return h;
        }
        else
        {
            return nullptr;
        }
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
            .baseaddr = &ret.baseaddr,
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
            ret.id = dl_id;
            ret.global = true;      // TODO: get this from kernel
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
    ret.id = -1;
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
    ret.id = -1;
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

int dlinfo(void *handle, int request, void *info)
{
    auto dl = getdl(handle);
    if(dl.fd < 0)
    {
        return -1;
    }

    switch(request)
    {
        case RTLD_DI_ORIGIN:
            strcpy((char *)info, dl.name);
            return 0;

        case RTLD_DI_PHDR:
            *(uintptr_t *)info = (uintptr_t)dl.eh + dl.eh->e_phoff;
            return (int)dl.eh->e_phnum;

        default:
            return -1;                
    }
}

static char _dlerror[] = "dlfcn.h functions are not supported";
char *dlerror()
{
    return _dlerror;
}

/* Get the first instance of a tag in the dynamic section */
static const ElfW(Dyn) *get_first_dyn(const ElfW(Phdr) *p_dyn, void *image, long id)
{
    if(!p_dyn)
        return nullptr;
    
    auto idx = 0u;
    while(idx < p_dyn->p_filesz)
    {
        const constexpr uintptr_t isize = sizeof(ElfW(Dyn));
        const auto cd = (const ElfW(Dyn) *)(
            (uintptr_t)image + p_dyn->p_offset + idx
        );

        if(cd->d_tag == id)
            return cd;
        if(cd->d_tag == DT_NULL)
            return nullptr;

        idx += isize;
    }
    return nullptr;
}

/* Iterate all tags of a type in the dynamic section and optionally call a function on them
    If id = -1, call on all tags */
static unsigned int iterate_dyn(const ElfW(Phdr) *p_dyn, void *image, long id,
    void *baseaddr,
    int (*ifunc)(const ElfW(Dyn) *, void *, void *))
{
    if(!p_dyn)
        return 0;

    unsigned int n_iter = 0;
    
    auto idx = 0u;
    while(idx < p_dyn->p_filesz)
    {
        const constexpr uintptr_t isize = sizeof(ElfW(Dyn));
        const auto cd = (const ElfW(Dyn) *)(
            (uintptr_t)image + p_dyn->p_offset + idx
        );

        if(cd->d_tag == id || id == -1)
        {
            n_iter++;
            if(ifunc)
            {
                if(ifunc(cd, image, baseaddr) != 0)
                    return n_iter;
            }
        }

        if(cd->d_tag == DT_NULL)
            return n_iter;

        idx += isize;
    }
    return n_iter;
}

/* Perform relocations on one particular table */
static int dlfcn_dorelas(const struct _dlinfo &dl,
    uintptr_t rela_table,
    uintptr_t rela_entsize,
    uintptr_t nrels,
    uintptr_t symtab,
    uintptr_t syment,
    uintptr_t strtab,
    ImageList &il)
{
    for(auto i = 0u; i < nrels; i++)
    {
        auto crel = (const ElfW(Rela) *)(
            (uintptr_t)dl.eh + rela_table + i * rela_entsize
        );

        [[maybe_unused]] auto r_sym = ELF_R_SYM(crel->r_info);
        [[maybe_unused]] auto r_type = ELF_R_TYPE(crel->r_info);
        uintptr_t target = crel->r_offset + (uintptr_t)dl.baseaddr;

        auto csym = (const ElfW(Sym) *)(
            (uintptr_t)dl.eh + symtab + r_sym * syment
        );

        auto cstr = (const char *)(
            (uintptr_t)dl.eh + strtab + csym->st_name
        );

#if DEBUG_DLOPEN
        fprintf(stderr, "dlopen(): reloc target: %p, type: %u, sym: %s, addend: %x\n",
            crel->r_offset, r_type, cstr, crel->r_addend);
#endif

        uintptr_t S = 0;

        if(cstr[0])
        {
            S = il.getsym(cstr, dl.id);
#if DEBUG_DLOPEN
            fprintf(stderr, "dlopen(): symbol %s = %p\n", cstr, S);
#endif
        }

        switch(r_type)
        {
            case R_AARCH64_ABS64:
                *(uint64_t *)target = S + crel->r_addend;
                break;
            case R_AARCH64_RELATIVE:
                *(uint64_t *)target = (uint64_t)dl.baseaddr + crel->r_addend;
                break;
            case R_AARCH64_GLOB_DAT:
                *(uint64_t *)target = S + crel->r_addend;
                break;
            case R_AARCH64_JUMP_SLOT:
                *(uint64_t *)target = S + crel->r_addend;
                break;
            case R_AARCH64_TLS_TPREL:
                fprintf(stderr, "warning: R_AARCH64_TLS_TPREL support not implemented\n");
                break;
            default:
                fprintf(stderr, "warning: relocation type %u not supported\n", r_type);
                break;
        }
    }
    return 0;
}

const ElfW(Phdr) *_dlinfo::pdyn()
{
    if(_p_dyn)
        return _p_dyn;
    
    // find dynamic section
    for(auto i = 0u; i < eh->e_phnum; i++)
    {
        const ElfW(Phdr) *cp = (const ElfW(Phdr) *)(
            (uintptr_t)eh + eh->e_phoff + i * eh->e_phentsize
        );
        if(cp->p_type == PT_DYNAMIC)
        {
            _p_dyn = cp;
            return cp;
        }
    }
    return nullptr;
}

static void reset_textrel(struct _dlinfo &dl)
{
#if DEBUG_DLOPEN
    fprintf(stderr, "dlopen: textrel encountered, remapping all exec segments as non-writeable\n");
#endif
    for(auto i = 0u; i < dl.eh->e_phnum; i++)
    {
        const auto *cp = (const ElfW(Phdr) *)((uintptr_t)dl.eh +
            i * dl.eh->e_phentsize);
        if(cp->p_type == PT_LOAD)
        {
            if((cp->p_flags & PF_X) && !(cp->p_flags & PF_W))
            {
                mprotect((void *)((uintptr_t)dl.baseaddr + cp->p_vaddr),
                    cp->p_memsz, PROT_READ | PROT_EXEC);
            }
        }
    }
}

/* Perform the actual load of a possibly dynamic image */
int dlfcn_loadimage(struct _dlinfo dl)
{
    if(!dl.eh)
    {
        return -1;
    }

    if(dl.eh->e_type == ET_EXEC)
    {
        // nothing to do
        return 0;
    }

    if(dl.eh->e_type != ET_REL && dl.eh->e_type != ET_DYN)
    {
        // warn here but pass
        fprintf(stderr, "dlfcn_loadimage: warning: e_type %u not supported\n", dl.eh->e_type);
        return 0;
    }

    const ElfW(Phdr) *p_dyn = nullptr;
    if(dl.eh->e_type == ET_DYN)
    {
        p_dyn = dl.pdyn();
    }

    bool symbolic = false;
    const ElfW(Dyn) *dt_hash = nullptr;
    const ElfW(Dyn) *dt_strtab = nullptr;
    const ElfW(Dyn) *dt_symtab = nullptr;
    const ElfW(Dyn) *dt_strsz = nullptr;
    const ElfW(Dyn) *dt_syment = nullptr;
    if(p_dyn)
    {
        // dump tags
#ifdef DEBUG_DLOPEN
        iterate_dyn(p_dyn, dl.eh, -1, dl.baseaddr, [](const ElfW(Dyn) *cd, void *img, void *ba) { 
            fprintf(stderr, "dlopen(): DYNAMIC: tag: %u, val: %p\n", cd->d_tag, cd->d_un.d_ptr);
            return 0;
        });
#endif

        // get mandatory sections
        dt_hash = get_first_dyn(p_dyn, dl.eh, DT_HASH);
        dt_strtab = get_first_dyn(p_dyn, dl.eh, DT_STRTAB);
        dt_symtab = get_first_dyn(p_dyn, dl.eh, DT_SYMTAB);
        dt_strsz = get_first_dyn(p_dyn, dl.eh, DT_STRSZ);
        dt_syment = get_first_dyn(p_dyn, dl.eh, DT_SYMENT);
        if(!dt_hash)
        {
            fprintf(stderr, "dlopen(): no DT_HASH entry\n");
            return -1;
        }
        if(!dt_strtab)
        {
            fprintf(stderr, "dlopen(): no DT_STRTAB entry\n");
            return -1;
        }
        if(!dt_symtab)
        {
            fprintf(stderr, "dlopen(): no DT_SYMTAB entry\n");
            return -1;
        }
        if(!dt_strsz)
        {
            fprintf(stderr, "dlopen(): no DT_STRSZ entry\n");
            return -1;
        }
        if(!dt_syment)
        {
            fprintf(stderr, "dlopen(): no DT_SYMENT entry\n");
            return -1;
        }
        if(get_first_dyn(p_dyn, dl.eh, DT_SYMBOLIC) != nullptr)
            symbolic = true;

        // load needed sections
        iterate_dyn(p_dyn, dl.eh, DT_NEEDED, (void *)dt_strtab->d_un.d_ptr, [](const ElfW(Dyn) *cd, void *img, void *strtab) {
            const char *needed_file = (const char *)(
                (uintptr_t)img + (uintptr_t)strtab + cd->d_un.d_ptr
            );
#ifdef DEBUG_DLOPEN
            fprintf(stderr, "dlopen(): loading dependency: %s\n", needed_file);
#endif
            auto dret = dlopen(needed_file, RTLD_NOW | RTLD_GLOBAL);
            if(!dret)
            {
                fprintf(stderr, "dlopen(): couldn't load dependency: %s\n", needed_file);
                return -1;
            }
            return 0;
        });
    }

    /* Build a list of searchable modules that we can check for symbols in.
        If we are symbolic, then add ourselves to the top of the list, else
        just iterate all loaded modules backwards i.e. so we have the most
        recent first.

      This is done as a class to allow RAII destruction if we fail at any point.
    */
    ImageList il(dl.id, symbolic);

    /* Decide if we need to make the .text section temporarily writeable */
    bool textrel = !p_dyn || get_first_dyn(p_dyn, dl.eh, DT_TEXTREL);
    if(textrel)
    {
#if DEBUG_DLOPEN
        fprintf(stderr, "dlopen: textrel encountered, mapping all segments as writeable\n");
#endif
        for(auto i = 0u; i < dl.eh->e_phnum; i++)
        {
            const auto *cp = (const ElfW(Phdr) *)((uintptr_t)dl.eh +
                dl.eh->e_phoff + i * dl.eh->e_phentsize);
            if(cp->p_type == PT_LOAD)
            {
                if((cp->p_flags & PF_X) && !(cp->p_flags & PF_W))
                {
                    mprotect((void *)((uintptr_t)dl.baseaddr + cp->p_vaddr),
                        cp->p_memsz, PROT_READ | PROT_WRITE);
                }
            }
        }
    }

    /* Perform the actual relocations */
    if(p_dyn)
    {
        auto dt_rela = get_first_dyn(p_dyn, dl.eh, DT_RELA);
        auto dt_relasz = get_first_dyn(p_dyn, dl.eh, DT_RELASZ);
        auto dt_relaent = get_first_dyn(p_dyn, dl.eh, DT_RELAENT);
        auto dt_relacount = get_first_dyn(p_dyn, dl.eh, DT_RELACOUNT);

        auto relaent = dt_relaent ? dt_relaent->d_un.d_val : sizeof(ElfW(Rela));

        if(dt_rela && dt_relasz)
        {
            auto nrels = dt_relasz->d_un.d_val / relaent;

            if(dt_relacount)
            {
                // TODO: optimize these because they are all local relocs of type R_ARCH64_RELATIVE or similar
            }

            if(dlfcn_dorelas(dl, dt_rela->d_un.d_val, relaent, nrels,
                dt_symtab->d_un.d_val, dt_syment->d_un.d_val, dt_strtab->d_un.d_val,
                il) != 0)
            {
                if(textrel) reset_textrel(dl);
                return -1;
            }
        }

        auto dt_jmprel = get_first_dyn(p_dyn, dl.eh, DT_JMPREL);
        auto dt_pltrel = get_first_dyn(p_dyn, dl.eh, DT_PLTREL);
        auto dt_pltrelsz = get_first_dyn(p_dyn, dl.eh, DT_PLTRELSZ);
        if(dt_jmprel && dt_pltrel && dt_pltrelsz)
        {
            if(dt_pltrel->d_un.d_val == DT_RELA)
            {
                auto nrels = dt_pltrelsz->d_un.d_val / relaent;

                if(dlfcn_dorelas(dl, dt_jmprel->d_un.d_val, relaent, nrels,
                    dt_symtab->d_un.d_val, dt_syment->d_un.d_val, dt_strtab->d_un.d_val,
                    il) != 0)
                {
                    if(textrel) reset_textrel(dl);
                    return -1;
                }
            }
        }
    }

    if(textrel) reset_textrel(dl);
    return 0;
}

uintptr_t _dlinfo::get_sym(const char *symname)
{
    if(fd < 0 || !eh)
        return 0;
    
    if(eh->e_type == ET_DYN)
    {
        if(!symtab)
        {
            const auto *p_dyn = pdyn();
            symtab = get_first_dyn(p_dyn, eh, DT_SYMTAB)->d_un.d_val;
            auto dt_syment = get_first_dyn(p_dyn, eh, DT_SYMENT);
            syment = dt_syment ? dt_syment->d_un.d_val : sizeof(ElfW(Sym));
            strtab = get_first_dyn(p_dyn, eh, DT_STRTAB)->d_un.d_val;
            hash = get_first_dyn(p_dyn, eh, DT_HASH)->d_un.d_val;

            const auto *htable = (const ElfW(Hash) *)(
                (uintptr_t)eh + hash
            );
            nsyms = htable->nchain;
        }

        // TODO: follow hash table for symbol lookup
    }
    else
    {
        if(!symtab)
        {
            // find a symtab table
            const auto *s_symtab = get_symtab(*this);
            symtab = s_symtab->sh_offset + s_symtab->sh_info * s_symtab->sh_entsize;
            syment = s_symtab->sh_entsize;
            nsyms = s_symtab->sh_size / syment - s_symtab->sh_info;
            strtab = ((const ElfW(Shdr) *)((uintptr_t)eh + eh->e_shoff +
                s_symtab->sh_link * eh->e_shentsize))->sh_offset;
        }
    }

#if DEBUG_DLOPEN
    fprintf(stderr, "search %s, symtab: %u, syment: %u, nsyms: %u, strtab: %u\n",
        this->name, symtab, syment, nsyms, strtab);
#endif

    // fallback to iterating the symbol table
    for(auto i = 0u; i < nsyms; i++)
    {
        const auto *csym = (const ElfW(Sym) *)((uintptr_t)eh + symtab + i * syment);
        const char *cstr = (const char *)((uintptr_t)eh + strtab + csym->st_name);
        if(!strcmp(symname, cstr))
        {
            return csym->st_value;
        }
    }

    return 0;
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
