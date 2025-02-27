/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

/* CPU feature detection for SDL */

#include "SDL.h"
#include "SDL_cpuinfo.h"

#if defined(__MACOSX__) && (defined(__ppc__) || defined(__ppc64__))
#include <sys/sysctl.h> /* For AltiVec check */
#elif defined(__OpenBSD__) && defined(__powerpc__)
#include <sys/param.h>
#include <sys/sysctl.h> /* For AltiVec check */
#include <machine/cpu.h>
#elif SDL_ALTIVEC_BLITTERS && HAVE_SETJMP
#include <signal.h>
#include <setjmp.h>
#endif

#if defined(__QNXNTO__)
#include <sys/syspage.h>
#endif

#if defined(__LINUX__) && defined(__arm__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>

/*#include <asm/hwcap.h>*/
#ifndef AT_HWCAP
#define AT_HWCAP 16
#endif
#ifndef AT_PLATFORM
#define AT_PLATFORM 15
#endif
#ifndef HWCAP_NEON
#define HWCAP_NEON (1 << 12)
#endif
#endif

#if defined(HAVE_GETAUXVAL) || defined(HAVE_ELF_AUX_INFO)
#include <sys/auxv.h>
#endif

#ifdef __RISCOS__
#include <kernel.h>
#include <swis.h>
#endif

#define CPU_HAS_RDTSC	0x00000001
#define CPU_HAS_MMX	0x00000002
#define CPU_HAS_MMXEXT	0x00000004
#define CPU_HAS_3DNOW	0x00000010
#define CPU_HAS_3DNOWEXT 0x00000020
#define CPU_HAS_SSE	0x00000040
#define CPU_HAS_SSE2	0x00000080
#define CPU_HAS_ALTIVEC	0x00000100
#define CPU_HAS_ARM_SIMD 0x00000200
#define CPU_HAS_NEON     0x00000400

#if SDL_ALTIVEC_BLITTERS && HAVE_SETJMP && !__MACOSX__ && !__OpenBSD__
/* This is the brute force way of detecting instruction sets...
   the idea is borrowed from the libmpeg2 library - thanks!
 */
static jmp_buf jmpbuf;
static void illegal_instruction(int sig)
{
	longjmp(jmpbuf, 1);
}
#endif /* HAVE_SETJMP */

static __inline__ int CPU_haveCPUID(void)
{
	int has_CPUID = 0;
#if defined(__GNUC__) && defined(__i386__)
	__asm__ (
"        pushfl                      # Get original EFLAGS             \n"
"        popl    %%eax                                                 \n"
"        movl    %%eax,%%ecx                                           \n"
"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
"        pushl   %%eax               # Save new EFLAGS value on stack  \n"
"        popfl                       # Replace current EFLAGS value    \n"
"        pushfl                      # Get new EFLAGS                  \n"
"        popl    %%eax               # Store new EFLAGS in EAX         \n"
"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
"        jz      1f                  # Processor=80486                 \n"
"        movl    $1,%0               # We have CPUID support           \n"
"1:                                                                    \n"
	: "=m" (has_CPUID)
	:
	: "%eax", "%ecx"
	);
#elif defined(__GNUC__) && defined(__x86_64__)
/* Technically, if this is being compiled under __x86_64__ then it has 
CPUid by definition.  But it's nice to be able to prove it.  :)      */
	__asm__ (
"        pushfq                      # Get original EFLAGS             \n"
"        popq    %%rax                                                 \n"
"        movq    %%rax,%%rcx                                           \n"
"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
"        pushq   %%rax               # Save new EFLAGS value on stack  \n"
"        popfq                       # Replace current EFLAGS value    \n"
"        pushfq                      # Get new EFLAGS                  \n"
"        popq    %%rax               # Store new EFLAGS in EAX         \n"
"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
"        jz      1f                  # Processor=80486                 \n"
"        movl    $1,%0               # We have CPUID support           \n"
"1:                                                                    \n"
	: "=m" (has_CPUID)
	:
	: "%rax", "%rcx"
	);
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
	__asm {
        pushfd                      ; Get original EFLAGS
        pop     eax
        mov     ecx, eax
        xor     eax, 200000h        ; Flip ID bit in EFLAGS
        push    eax                 ; Save new EFLAGS value on stack
        popfd                       ; Replace current EFLAGS value
        pushfd                      ; Get new EFLAGS
        pop     eax                 ; Store new EFLAGS in EAX
        xor     eax, ecx            ; Can not toggle ID bit,
        jz      done                ; Processor=80486
        mov     has_CPUID,1         ; We have CPUID support
done:
	}
#elif defined(__sun) && defined(__i386)
	__asm (
"       pushfl                 \n"
"	popl    %eax           \n"
"	movl    %eax,%ecx      \n"
"	xorl    $0x200000,%eax \n"
"	pushl   %eax           \n"
"	popfl                  \n"
"	pushfl                 \n"
"	popl    %eax           \n"
"	xorl    %ecx,%eax      \n"
"	jz      1f             \n"
"	movl    $1,-8(%ebp)    \n"
"1:                            \n"
	);
#elif defined(__sun) && defined(__amd64)
	__asm (
"       pushfq                 \n"
"       popq    %rax           \n"
"       movq    %rax,%rcx      \n"
"       xorl    $0x200000,%eax \n"
"       pushq   %rax           \n"
"       popfq                  \n"
"       pushfq                 \n"
"       popq    %rax           \n"
"       xorl    %ecx,%eax      \n"
"       jz      1f             \n"
"       movl    $1,-8(%rbp)    \n"
"1:                            \n"
	);
#endif
	return has_CPUID;
}

static __inline__ int CPU_getCPUIDFeatures(void)
{
	int features = 0;
#if defined(__GNUC__) && defined(__i386__)
	__asm__ (
"        xorl    %%eax,%%eax         # Set up for CPUID instruction    \n"
"        pushl   %%ebx                                                 \n"
"        cpuid                       # Get and save vendor ID          \n"
"        popl    %%ebx                                                 \n"
"        cmpl    $1,%%eax            # Make sure 1 is valid input for CPUID\n"
"        jl      1f                  # We dont have the CPUID instruction\n"
"        xorl    %%eax,%%eax                                           \n"
"        incl    %%eax                                                 \n"
"        pushl   %%ebx                                                 \n"
"        cpuid                       # Get family/model/stepping/features\n"
"        popl    %%ebx                                                 \n"
"        movl    %%edx,%0                                              \n"
"1:                                                                    \n"
	: "=m" (features)
	:
	: "%eax", "%ecx", "%edx"
	);
#elif defined(__GNUC__) && defined(__x86_64__)
	__asm__ (
"        xorl    %%eax,%%eax         # Set up for CPUID instruction    \n"
"        pushq   %%rbx                                                 \n"
"        cpuid                       # Get and save vendor ID          \n"
"        popq    %%rbx                                                 \n"
"        cmpl    $1,%%eax            # Make sure 1 is valid input for CPUID\n"
"        jl      1f                  # We dont have the CPUID instruction\n"
"        xorl    %%eax,%%eax                                           \n"
"        incl    %%eax                                                 \n"
"        pushq   %%rbx                                                 \n"
"        cpuid                       # Get family/model/stepping/features\n"
"        popq    %%rbx                                                 \n"
"        movl    %%edx,%0                                              \n"
"1:                                                                    \n"
	: "=m" (features)
	:
	: "%rax", "%rcx", "%rdx"
	);
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
	__asm {
        xor     eax, eax            ; Set up for CPUID instruction
        push    ebx
        cpuid                       ; Get and save vendor ID
        pop     ebx
        cmp     eax, 1              ; Make sure 1 is valid input for CPUID
        jl      done                ; We dont have the CPUID instruction
        xor     eax, eax
        inc     eax
        push    ebx
        cpuid                       ; Get family/model/stepping/features
        pop     ebx
        mov     features, edx
done:
	}
#elif defined(__sun) && (defined(__i386) || defined(__amd64))
	__asm(
"        xorl    %eax,%eax         \n"
"        pushl   %ebx              \n"
"        cpuid                     \n"
"        popl    %ebx              \n"
"        cmpl    $1,%eax           \n"
"        jl      1f                \n"
"        xorl    %eax,%eax         \n"
"        incl    %eax              \n"
"        pushl   %ebx              \n"
"        cpuid                     \n"
"        popl    %ebx              \n"
#ifdef __i386
"        movl    %edx,-8(%ebp)     \n"
#else
"        movl    %edx,-8(%rbp)     \n"
#endif
"1:                                \n"
	    );
#endif
	return features;
}

static __inline__ int CPU_getCPUIDFeaturesExt(void)
{
	int features = 0;
#if defined(__GNUC__) && defined(__i386__)
	__asm__ (
"        movl    $0x80000000,%%eax   # Query for extended functions    \n"
"        pushl   %%ebx                                                 \n"
"        cpuid                       # Get extended function limit     \n"
"        popl    %%ebx                                                 \n"
"        cmpl    $0x80000001,%%eax                                     \n"
"        jl      1f                  # Nope, we dont have function 800000001h\n"
"        movl    $0x80000001,%%eax   # Setup extended function 800000001h\n"
"        pushl   %%ebx                                                 \n"
"        cpuid                       # and get the information         \n"
"        popl    %%ebx                                                 \n"
"        movl    %%edx,%0                                              \n"
"1:                                                                    \n"
	: "=m" (features)
	:
	: "%eax", "%ecx", "%edx"
	);
#elif defined(__GNUC__) && defined (__x86_64__)
	__asm__ (
"        movl    $0x80000000,%%eax   # Query for extended functions    \n"
"        pushq   %%rbx                                                 \n"
"        cpuid                       # Get extended function limit     \n"
"        popq    %%rbx                                                 \n"
"        cmpl    $0x80000001,%%eax                                     \n"
"        jl      1f                  # Nope, we dont have function 800000001h\n"
"        movl    $0x80000001,%%eax   # Setup extended function 800000001h\n"
"        pushq   %%rbx                                                 \n"
"        cpuid                       # and get the information         \n"
"        popq    %%rbx                                                 \n"
"        movl    %%edx,%0                                              \n"
"1:                                                                    \n"
	: "=m" (features)
	:
	: "%rax", "%rcx", "%rdx"
	);
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
	__asm {
        mov     eax,80000000h       ; Query for extended functions
        push    ebx
        cpuid                       ; Get extended function limit
        pop     ebx
        cmp     eax,80000001h
        jl      done                ; Nope, we dont have function 800000001h
        mov     eax,80000001h       ; Setup extended function 800000001h
        push    ebx
        cpuid                       ; and get the information
        pop     ebx
        mov     features,edx
done:
	}
#elif defined(__sun) && (defined(__i386) || defined(__amd64))
	__asm (
"        movl    $0x80000000,%eax \n"
"        pushl   %ebx             \n"
"        cpuid                    \n"
"        popl    %ebx             \n"
"        cmpl    $0x80000001,%eax \n"
"        jl      1f               \n"
"        movl    $0x80000001,%eax \n"
"        pushl   %ebx             \n"
"        cpuid                    \n"
"        popl    %ebx             \n"
#ifdef __i386
"        movl    %edx,-8(%ebp)   \n"
#else
"        movl    %edx,-8(%rbp)   \n"
#endif
"1:                               \n"
	);
#endif
	return features;
}

static __inline__ int CPU_haveRDTSC(void)
{
	if ( CPU_haveCPUID() ) {
		return (CPU_getCPUIDFeatures() & 0x00000010);
	}
	return 0;
}

static __inline__ int CPU_haveMMX(void)
{
	if ( CPU_haveCPUID() ) {
		return (CPU_getCPUIDFeatures() & 0x00800000);
	}
	return 0;
}

static __inline__ int CPU_haveMMXExt(void)
{
	if ( CPU_haveCPUID() ) {
		return (CPU_getCPUIDFeaturesExt() & 0x00400000);
	}
	return 0;
}

static __inline__ int CPU_have3DNow(void)
{
	if ( CPU_haveCPUID() ) {
		return (CPU_getCPUIDFeaturesExt() & 0x80000000);
	}
	return 0;
}

static __inline__ int CPU_have3DNowExt(void)
{
	if ( CPU_haveCPUID() ) {
		return (CPU_getCPUIDFeaturesExt() & 0x40000000);
	}
	return 0;
}

static __inline__ int CPU_haveSSE(void)
{
	if ( CPU_haveCPUID() ) {
		return (CPU_getCPUIDFeatures() & 0x02000000);
	}
	return 0;
}

static __inline__ int CPU_haveSSE2(void)
{
	if ( CPU_haveCPUID() ) {
		return (CPU_getCPUIDFeatures() & 0x04000000);
	}
	return 0;
}

static __inline__ int CPU_haveAltiVec(void)
{
	volatile int altivec = 0;
#if (defined(__MACOSX__) && (defined(__ppc__) || defined(__ppc64__))) || (defined(__OpenBSD__) && defined(__powerpc__))
# ifdef __OpenBSD__
	int selectors[2] = { CTL_MACHDEP, CPU_ALTIVEC };
# else
	int selectors[2] = { CTL_HW, HW_VECTORUNIT };
# endif
	int hasVectorUnit = 0; 
	size_t length = sizeof(hasVectorUnit); 
	int error = sysctl(selectors, 2, &hasVectorUnit, &length, NULL, 0); 
	if( 0 == error )
		altivec = (hasVectorUnit != 0); 
#elif SDL_ALTIVEC_BLITTERS && HAVE_SETJMP
	void (*handler)(int sig);
	handler = signal(SIGILL, illegal_instruction);
	if ( setjmp(jmpbuf) == 0 ) {
		asm volatile ("mtspr 256, %0\n\t"
			      "vand %%v0, %%v0, %%v0"
			      :
			      : "r" (-1));
		altivec = 1;
	}
	signal(SIGILL, handler);
#endif
	return altivec; 
}

/* ARM SIMD / NEON detection
 * Not public - for internal ARM 32 bit blitters' use only
 */
#if !defined(__arm__)  /* not an ARM CPU at all. */
static int
CPU_haveARMSIMD(void)
{
	return 0;
}

#elif defined(_WIN32_WCE)
static int
CPU_haveARMSIMD(void)
{
	return 0;
}

#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 6)
static int
CPU_haveARMSIMD(void)
{
	return 1;
}

#elif defined(__LINUX__)
static __inline__ int CPU_haveARMSIMD(void)
{
	int arm_simd = 0;
	int fd;

	fd = open("/proc/self/auxv", O_RDONLY);
	if (fd >= 0)
	{
		Elf32_auxv_t aux;
		while (read(fd, &aux, sizeof aux) == sizeof aux)
		{
			if (aux.a_type == AT_PLATFORM)
			{
				const char *plat = (const char *) aux.a_un.a_val;
				if (plat) {
					arm_simd = strncmp(plat, "v6l", 3) == 0 ||
						strncmp(plat, "v7l", 3) == 0;
				}
			}
		}
		close(fd);
	}
	return arm_simd;
}

#elif defined(__RISCOS__)
static __inline__ int CPU_haveARMSIMD(void)
{
	_kernel_swi_regs regs;
	regs.r[0] = 0;
	if (_kernel_swi(OS_PlatformFeatures, &regs, &regs) != NULL)
		return 0;

	if (!(regs.r[0] & (1<<31)))
		return 0;

	regs.r[0] = 34;
	regs.r[1] = 29;
	if (_kernel_swi(OS_PlatformFeatures, &regs, &regs) != NULL)
		return 0;

	return regs.r[0];
}

#else
static __inline__ int CPU_haveARMSIMD(void)
{
#warning SDL_HasARMSIMD is not implemented for this ARM platform. Write me.
	return 0;
}

#endif

#if defined(__LINUX__) && defined(__arm__) && !defined(HAVE_GETAUXVAL)
static __inline__  int readProcAuxvForNeon(void)
{
	int neon = 0;
	int fd;

	fd = open("/proc/self/auxv", O_RDONLY);
	if (fd >= 0)
	{
		Elf32_auxv_t aux;
		while (read(fd, &aux, sizeof (aux)) == sizeof (aux)) {
			if (aux.a_type == AT_HWCAP) {
				neon = (aux.a_un.a_val & HWCAP_NEON) == HWCAP_NEON;
				break;
			}
		}
		close(fd);
	}
	return neon;
}
#endif

static __inline__ int CPU_haveNEON(void)
{
#if !defined(__arm__)  /* not an ARM CPU at all. */
	return 0;
#elif defined(_WIN32_WCE)
	return 0;
/* The way you detect NEON is a privileged instruction on ARM, so you have
   query the OS kernel in a platform-specific way. :/ */
#elif defined(__ARM_ARCH) && (__ARM_ARCH >= 8)
	return 1;  /* ARMv8 always has non-optional NEON support. */
#elif defined(__APPLE__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
	/* (note that sysctlbyname("hw.optional.neon") doesn't work!) */
	return 1;  /* all Apple ARMv7 chips and later have NEON. */
#elif defined(__APPLE__)
	return 0;  /* assume anything else from Apple doesn't have NEON. */
#elif defined(__OpenBSD__)
	return 1;  /* OpenBSD only supports ARMv7 CPUs that have NEON. */
#elif defined(HAVE_ELF_AUX_INFO)
	unsigned long hasneon = 0;
	if (elf_aux_info(AT_HWCAP, (void *)&hasneon, (int)sizeof(hasneon)) != 0)
		return 0;
	return ((hasneon & HWCAP_NEON) == HWCAP_NEON);
#elif defined(__QNXNTO__)
	return SYSPAGE_ENTRY(cpuinfo)->flags & ARM_CPU_FLAG_NEON;
#elif defined(__LINUX__) && defined(HAVE_GETAUXVAL)
	return ((getauxval(AT_HWCAP) & HWCAP_NEON) == HWCAP_NEON);
#elif defined(__LINUX__)
	return readProcAuxvForNeon();
#elif defined(__RISCOS__)
	/* Use the VFPSupport_Features SWI to access the MVFR registers */
	{
		_kernel_swi_regs regs;
		regs.r[0] = 0;
		if (_kernel_swi(VFPSupport_Features, &regs, &regs) == NULL) {
		if ((regs.r[2] & 0xFFF000) == 0x111000) {
			return 1;
			}
		}
		return 0;
	}
#elif defined(__GAMEKID__)
	return 0;
#else
#warning SDL_HasNEON is not implemented for this ARM platform. Write me.
	return 0;
#endif
}

static Uint32 SDL_CPUFeatures = 0xFFFFFFFF;

static Uint32 SDL_GetCPUFeatures(void)
{
	if ( SDL_CPUFeatures == 0xFFFFFFFF ) {
		SDL_CPUFeatures = 0;
		if ( CPU_haveRDTSC() ) {
			SDL_CPUFeatures |= CPU_HAS_RDTSC;
		}
		if ( CPU_haveMMX() ) {
			SDL_CPUFeatures |= CPU_HAS_MMX;
		}
		if ( CPU_haveMMXExt() ) {
			SDL_CPUFeatures |= CPU_HAS_MMXEXT;
		}
		if ( CPU_have3DNow() ) {
			SDL_CPUFeatures |= CPU_HAS_3DNOW;
		}
		if ( CPU_have3DNowExt() ) {
			SDL_CPUFeatures |= CPU_HAS_3DNOWEXT;
		}
		if ( CPU_haveSSE() ) {
			SDL_CPUFeatures |= CPU_HAS_SSE;
		}
		if ( CPU_haveSSE2() ) {
			SDL_CPUFeatures |= CPU_HAS_SSE2;
		}
		if ( CPU_haveAltiVec() ) {
			SDL_CPUFeatures |= CPU_HAS_ALTIVEC;
		}
		if ( CPU_haveARMSIMD() ) {
			SDL_CPUFeatures |= CPU_HAS_ARM_SIMD;
		}
		if ( CPU_haveNEON() ) {
			SDL_CPUFeatures |= CPU_HAS_NEON;
		}
	}
	return SDL_CPUFeatures;
}

SDL_bool SDL_HasRDTSC(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_RDTSC ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

SDL_bool SDL_HasMMX(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_MMX ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

SDL_bool SDL_HasMMXExt(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_MMXEXT ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

SDL_bool SDL_Has3DNow(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_3DNOW ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

SDL_bool SDL_Has3DNowExt(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_3DNOWEXT ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

SDL_bool SDL_HasSSE(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_SSE ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

SDL_bool SDL_HasSSE2(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_SSE2 ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

SDL_bool SDL_HasAltiVec(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_ALTIVEC ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

SDL_bool SDL_HasARMSIMD(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_ARM_SIMD ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

SDL_bool SDL_HasNEON(void)
{
	if ( SDL_GetCPUFeatures() & CPU_HAS_NEON ) {
		return SDL_TRUE;
	}
	return SDL_FALSE;
}

#ifdef TEST_MAIN

#include <stdio.h>

int main()
{
	printf("RDTSC: %d\n", SDL_HasRDTSC());
	printf("MMX: %d\n", SDL_HasMMX());
	printf("MMXExt: %d\n", SDL_HasMMXExt());
	printf("3DNow: %d\n", SDL_Has3DNow());
	printf("3DNowExt: %d\n", SDL_Has3DNowExt());
	printf("SSE: %d\n", SDL_HasSSE());
	printf("SSE2: %d\n", SDL_HasSSE2());
	printf("AltiVec: %d\n", SDL_HasAltiVec());
	printf("ARM SIMD: %d\n", SDL_HasARMSIMD());
	printf("NEON: %d\n", SDL_HasNEON());
	return 0;
}

#endif /* TEST_MAIN */
