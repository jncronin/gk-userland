/* crt0 for GKv4
    The majority of stack/argument setup is performed for us by GK itself.
    We simply need to call the .init functions here, and ensure .fini is called
    at the end. */

.cpu cortex-a35

.section .text._start
.global _start
.type _start,%function
_start:
    // initialise recursive locks in newlib
    bl __gk_newlib_init_locks
    
    // get [ argc, argv ], put in x24/25
    bl __gk_init_args
    mov x24, x0
    mov x25, x1

    // get envp, put in environs and x26
    bl __gk_init_env
    mov x26, x0

    ldr x0, =__libc_fini_array
    bl atexit

    bl __libc_init_array

    mov x0, x24
    mov x1, x25
    mov x2, x26

    bl main

    bl exit

.L0: b .L0

.size _start,.-_start
