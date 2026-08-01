#ifndef SYS__ELF64_H
#define SYS__ELF64_H

#include <stdint.h>
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

#define __ELF_NATIVE_CLASS 64

typedef struct
{
    unsigned char e_ident[16]; /* ELF identification */
    Elf64_Half e_type; /* Object file type */
    Elf64_Half e_machine; /* Machine type */
    Elf64_Word e_version; /* Object file version */
    Elf64_Addr e_entry; /* Entry point address */
    Elf64_Off e_phoff; /* Program header offset */
    Elf64_Off e_shoff; /* Section header offset */
    Elf64_Word e_flags; /* Processor-specific flags */
    Elf64_Half e_ehsize; /* ELF header size */
    Elf64_Half e_phentsize; /* Size of program header entry */
    Elf64_Half e_phnum; /* Number of program header entries */
    Elf64_Half e_shentsize; /* Size of section header entry */
    Elf64_Half e_shnum; /* Number of section header entries */
    Elf64_Half e_shstrndx; /* Section name string table index */
} __attribute__((packed)) Elf64_Ehdr;

typedef struct
{
    Elf64_Word p_type; /* Type of segment */
    Elf64_Word p_flags; /* Segment attributes */
    Elf64_Off p_offset; /* Offset in file */
    Elf64_Addr p_vaddr; /* Virtual address in memory */
    Elf64_Addr p_paddr; /* Reserved */
    Elf64_Xword p_filesz; /* Size of segment in file */
    Elf64_Xword p_memsz; /* Size of segment in memory */
    Elf64_Xword p_align; /* Alignment of segment */
} __attribute__((packed)) Elf64_Phdr;

typedef struct
{
    Elf64_Word sh_name; /* Section name */
    Elf64_Word sh_type; /* Section type */
    Elf64_Xword sh_flags; /* Section attributes */
    Elf64_Addr sh_addr; /* Virtual address in memory */
    Elf64_Off sh_offset; /* Offset in file */
    Elf64_Xword sh_size; /* Size of section */
    Elf64_Word sh_link; /* Link to other section */
    Elf64_Word sh_info; /* Miscellaneous information */
    Elf64_Xword sh_addralign; /* Address alignment boundary */
    Elf64_Xword sh_entsize; /* Size of entries, if section has table */
} __attribute__((packed)) Elf64_Shdr;

typedef struct
{
    Elf64_Word st_name; /* Symbol name */
    unsigned char st_info; /* Type and Binding attributes */
    unsigned char st_other; /* Reserved */
    Elf64_Half st_shndx; /* Section table index */
    Elf64_Addr st_value; /* Symbol value */
    Elf64_Xword st_size; /* Size of object (e.g., common) */
} __attribute__((packed)) Elf64_Sym;

typedef struct 
{
    Elf64_Word n_namesz;
    Elf64_Word n_descsz;
    Elf64_Word n_type;
} __attribute__((packed)) Elf64_Nhdr;

typedef struct
{
    Elf64_Sxword d_tag;
    union
    {
        Elf64_Xword d_val;
        Elf64_Addr d_ptr;
    } d_un;
} __attribute__((packed)) Elf64_Dyn;

typedef struct
{
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
    Elf64_Sxword r_addend;
} __attribute__((packed)) Elf64_Rela;

typedef struct
{
    Elf64_Word nbucket;
    Elf64_Word nchain;
    Elf64_Word bucket[];
} __attribute__((packed)) Elf64_Hash;

#define EM_AARCH64      0xb7

#define EI_CLASS        4
#define ELFCLASS64      2

#define EI_DATA         5
#define ELFDATA2LSB     1

#define ET_NONE         0
#define ET_REL          1
#define ET_EXEC         2
#define ET_DYN          3
#define ET_CORE         4

#define PT_NULL         0
#define PT_LOAD         1
#define PT_DYNAMIC      2
#define PT_INTERP       3
#define PT_NOTE         4
#define PT_SHLIB        5
#define PT_PHDR         6
#define PT_LOPROC       0x70000000
#define PT_HIPROC       0x7fffffff

#define PF_X            1
#define PF_W            2

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_LOOS 0x60000000
#define SHT_HIOS 0x6FFFFFFF
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7FFFFFFF

#define DT_NULL                 0
#define DT_NEEDED               1
#define DT_PLTRELSZ             2
#define DT_PLTGOT               3
#define DT_HASH                 4
#define DT_STRTAB               5
#define DT_SYMTAB               6
#define DT_RELA                 7
#define DT_RELASZ               8
#define DT_RELAENT              9
#define DT_STRSZ                10
#define DT_SYMENT               11
#define DT_INIT                 12
#define DT_FINI                 13
#define DT_SONAME               14
#define DT_RPATH                15
#define DT_SYMBOLIC             16
#define DT_REL                  17
#define DT_RELSZ                18
#define DT_RELENT               19
#define DT_PLTREL               20
#define DT_DEBUG                21
#define DT_TEXTREL              22
#define DT_JMPREL               23
#define DT_BIND_NOW             24
#define DT_INIT_ARRAY           25
#define DT_FINI_ARRAY           26
#define DT_INIT_ARRAYSZ         27
#define DT_FINI_ARRAYSZ         28
#define DT_LOOS                 0x60000000
#define DT_HIOS                 0x6fffffff
#define DT_LOPROC               0x70000000
#define DT_HIPROC               0x7fffffff

#define DT_RELACOUNT	        0x6ffffff9
#define DT_RELCOUNT	            0x6ffffffa

#define ELF32_R_SYM(val)		((val) >> 8)
#define ELF32_R_TYPE(val)		((val) & 0xff)
#define ELF32_R_INFO(sym, type)		(((sym) << 8) + ((type) & 0xff))

#define ELF64_R_SYM(i)			((i) >> 32)
#define ELF64_R_TYPE(i)			((i) & 0xffffffff)
#define ELF64_R_INFO(sym,type)		((((Elf64_Xword) (sym)) << 32) + (type))

#if __ELF_NATIVE_CLASS == 64
#define ELF_R_SYM(i) ELF64_R_SYM(i)
#define ELF_R_TYPE(i) ELF64_R_TYPE(i)
#define ELF_R_INFO(sym,type) ELF64_R_INFO(sym,type)
#elif __ELF_NATIVE_CLASS == 32
#define ELF_R_SYM(i) ELF32_R_SYM(i)
#define ELF_R_TYPE(i) ELF32_R_TYPE(i)
#define ELF_R_INFO(sym,type) ELF32_R_INFO(sym,type)
#endif

#define R_AARCH64_RELATIVE          1027
#define R_AARCH64_ABS64             257
#define R_AARCH64_TLS_TPREL         1030
#define R_AARCH64_JUMP_SLOT         1026
#define R_AARCH64_GLOB_DAT          1025

#endif