/* crt0 for GKv4
    The majority of stack/argument setup is performed for us by GK itself.
    We simply need to call the .init functions here, and ensure .fini is called
    at the end. */

.cpu cortex-a35

.section .text._start
.global _start
.type _start,%function
_start:
    mov x24, x0
    mov x25, x1

    //bl prvHeapInit

    bl __gk_init_env

    ldr x0, =__libc_fini_array
    bl atexit

    bl __libc_init_array

    mov x0, x24
    mov x1, x25

    bl main

    bl exit

.L0: b .L0

.size _start,.-_start
