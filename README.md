# Userland for gkos #

This is a collection of build tools and userspace libraries for gkos (https://github.com/jncronin/gk), a relatively feature-complete multitasking operating system for the STM32H747 microcontroller.  Userspace applications on gkos only run on the M7 core (the M4 core is reserved for operating system purposes) and therefore all code here is optimised for the M7.

This repository contains sources for many other projects each under their own respective licences.

In places they have been modified for gkos (e.g. hardware blitter support for SDL) and such locations are highlighted in the source.


## Building ##

gkos is not self-hosting, i.e. all compilation must be done on a host machine targeting the microcontroller.  Two distinct directory structures are required on the host machine.

1) $TOOLSDIR - this contains binaries (e.g. compiler, assembler, linker) which run on the host machine but target gkos.
2) $SYSROOT - this contains libraries compiled for gkos which will eventually be combined into static binaries to run on gkos.

To generate the toolchain and libc/libstdc++ for gkos:

./build-toolchain.sh


To generate the supplementary libraries:

./build-libraries.sh


Note both scripts depend on the TOOLSDIR/SYSROOT environment variables.  If not set they will choose a location in the user's home directory (see scripts for examples).


## Nuances ##

The arm-none-gkos- compilers/linker default with several options required to run on gkos.  Given gkos does not have a MMU, but does support multitasking, several tricks are needed.  All binaries are statically linked to address 0 and all relocations are emitted in the binary.  Thus, these can be patched up if needed when loaded by gkos (see gkos elf loading docs for further information).  This static linking has the requirement that if any library is changed then all dependent binaries are required to be relinked.

gkos supports the GCC \_\_attribute\_\_((hot)) label on functions.  This causes these functions to be placed in a special part of the binary which, when loaded by gkos, is preferentially placed in fast interal RAM, and ideally ITCM.  This is already the case, for example, for the Mesa software rasterizer.

gkos packages are in .tar.gz format.  For the provided cmake toolchain file (toolchain-gkos.cmake), appropriate CPack directives exist to create these.  Simply add "include(CPack)" to the end of any CMakeLists.txt and these will be used automatically.

