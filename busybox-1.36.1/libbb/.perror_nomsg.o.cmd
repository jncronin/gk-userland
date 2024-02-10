cmd_libbb/perror_nomsg.o := arm-none-eabi-gcc -Wp,-MD,libbb/.perror_nomsg.o.d  -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -DBB_VER='"1.36.1"' -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffast-math --specs=nano.specs -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS -I/home/jncronin/src/gk/include -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wno-format-security -Wdeclaration-after-statement -Wold-style-definition -finline-limit=0 -fno-builtin-strlen -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-builtin-printf -Os    -DKBUILD_BASENAME='"perror_nomsg"'  -DKBUILD_MODNAME='"perror_nomsg"' -c -o libbb/perror_nomsg.o libbb/perror_nomsg.c

deps_libbb/perror_nomsg.o := \
  libbb/perror_nomsg.c \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config/big/endian.h) \
    $(wildcard include/config/little/endian.h) \
    $(wildcard include/config/nommu.h) \
  /usr/lib/gcc/arm-none-eabi/10.3.1/include-fixed/limits.h \
  /home/jncronin/src/gk/include/byteswap.h \
  /home/jncronin/src/gk/include/bits/byteswap.h \
  /home/jncronin/src/gk/include/endian.h \
  /home/jncronin/src/gk/include/features.h \
  /usr/include/newlib/sys/cdefs.h \
  /usr/include/newlib/machine/_default_types.h \
  /usr/include/newlib/sys/features.h \
  /usr/include/newlib/_newlib_version.h \
  /usr/lib/gcc/arm-none-eabi/10.3.1/include/stddef.h \
  /home/jncronin/src/gk/include/bits/endian.h \
  /usr/lib/gcc/arm-none-eabi/10.3.1/include/stdint.h \
  /usr/lib/gcc/arm-none-eabi/10.3.1/include/stdbool.h \
  /usr/include/newlib/unistd.h \
  /usr/include/newlib/sys/unistd.h \
  /usr/include/newlib/_ansi.h \
  /usr/include/newlib/nano/newlib.h \
  /usr/include/newlib/sys/config.h \
  /usr/include/newlib/machine/ieeefp.h \
  /usr/include/newlib/sys/types.h \
  /usr/include/newlib/machine/_types.h \
  /usr/include/newlib/sys/_types.h \
  /usr/include/newlib/sys/_stdint.h \
  /usr/include/newlib/machine/endian.h \
  /usr/include/newlib/machine/_endian.h \
  /usr/include/newlib/sys/select.h \
  /usr/include/newlib/sys/_sigset.h \
  /usr/include/newlib/sys/_timeval.h \
  /usr/include/newlib/sys/timespec.h \
  /usr/include/newlib/sys/_timespec.h \
  /usr/include/newlib/sys/_pthreadtypes.h \
  /usr/include/newlib/sys/sched.h \
  /usr/include/newlib/machine/types.h \

libbb/perror_nomsg.o: $(deps_libbb/perror_nomsg.o)

$(deps_libbb/perror_nomsg.o):
