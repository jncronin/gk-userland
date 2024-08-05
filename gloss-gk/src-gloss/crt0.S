/* crt0 for GK
    The majority of stack/argument setup is performed for us by GK itself.
    We simply need to call the .init functions here, and ensure .fini is called
    at the end. */

.align 4
.thumb

.global _mainCRTStartup
_mainCRTStartup:
    mov r4, r0
    mov r5, r1

    ldr r0, =.L_fini_addr
    bl atexit

    bl __libc_init_array

    mov r0, r4
    mov r1, r5

    bl main

    bl exit

.L0: b .L0

.L_fini_addr: .word __libc_fini_array
