cmd_libbb/fgets_str.o := arm-none-eabi-gcc -Wp,-MD,libbb/.fgets_str.o.d  -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -DBB_VER='"1.36.1"' -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffast-math --specs=nano.specs -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS -I/home/jncronin/src/gk/include -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wno-format-security -Wdeclaration-after-statement -Wold-style-definition -finline-limit=0 -fno-builtin-strlen -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-builtin-printf -Os    -DKBUILD_BASENAME='"fgets_str"'  -DKBUILD_MODNAME='"fgets_str"' -c -o libbb/fgets_str.o libbb/fgets_str.c

deps_libbb/fgets_str.o := \
  libbb/fgets_str.c \
  include/libbb.h \
    $(wildcard include/config/feature/shadowpasswds.h) \
    $(wildcard include/config/use/bb/shadow.h) \
    $(wildcard include/config/selinux.h) \
    $(wildcard include/config/feature/utmp.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/use/bb/pwd/grp.h) \
    $(wildcard include/config/lfs.h) \
    $(wildcard include/config/feature/buffers/go/on/stack.h) \
    $(wildcard include/config/feature/buffers/go/in/bss.h) \
    $(wildcard include/config/extra/cflags.h) \
    $(wildcard include/config/variable/arch/pagesize.h) \
    $(wildcard include/config/feature/verbose.h) \
    $(wildcard include/config/feature/etc/services.h) \
    $(wildcard include/config/feature/ipv6.h) \
    $(wildcard include/config/feature/seamless/xz.h) \
    $(wildcard include/config/feature/seamless/lzma.h) \
    $(wildcard include/config/feature/seamless/bz2.h) \
    $(wildcard include/config/feature/seamless/gz.h) \
    $(wildcard include/config/feature/seamless/z.h) \
    $(wildcard include/config/float/duration.h) \
    $(wildcard include/config/feature/check/names.h) \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/long/opts.h) \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/feature/syslog/info.h) \
    $(wildcard include/config/warn/simple/msg.h) \
    $(wildcard include/config/feature/individual.h) \
    $(wildcard include/config/shell/ash.h) \
    $(wildcard include/config/shell/hush.h) \
    $(wildcard include/config/echo.h) \
    $(wildcard include/config/sleep.h) \
    $(wildcard include/config/printf.h) \
    $(wildcard include/config/test.h) \
    $(wildcard include/config/test1.h) \
    $(wildcard include/config/test2.h) \
    $(wildcard include/config/kill.h) \
    $(wildcard include/config/killall.h) \
    $(wildcard include/config/killall5.h) \
    $(wildcard include/config/chown.h) \
    $(wildcard include/config/ls.h) \
    $(wildcard include/config/xxx.h) \
    $(wildcard include/config/route.h) \
    $(wildcard include/config/feature/hwib.h) \
    $(wildcard include/config/desktop.h) \
    $(wildcard include/config/feature/crond/d.h) \
    $(wildcard include/config/feature/setpriv/capabilities.h) \
    $(wildcard include/config/run/init.h) \
    $(wildcard include/config/feature/securetty.h) \
    $(wildcard include/config/pam.h) \
    $(wildcard include/config/use/bb/crypt.h) \
    $(wildcard include/config/feature/adduser/to/group.h) \
    $(wildcard include/config/feature/del/user/from/group.h) \
    $(wildcard include/config/ioctl/hex2str/error.h) \
    $(wildcard include/config/feature/editing.h) \
    $(wildcard include/config/feature/editing/history.h) \
    $(wildcard include/config/feature/tab/completion.h) \
    $(wildcard include/config/feature/username/completion.h) \
    $(wildcard include/config/feature/editing/fancy/prompt.h) \
    $(wildcard include/config/feature/editing/savehistory.h) \
    $(wildcard include/config/feature/editing/vi.h) \
    $(wildcard include/config/feature/editing/save/on/exit.h) \
    $(wildcard include/config/pmap.h) \
    $(wildcard include/config/feature/show/threads.h) \
    $(wildcard include/config/feature/ps/additional/columns.h) \
    $(wildcard include/config/feature/topmem.h) \
    $(wildcard include/config/feature/top/smp/process.h) \
    $(wildcard include/config/pgrep.h) \
    $(wildcard include/config/pkill.h) \
    $(wildcard include/config/pidof.h) \
    $(wildcard include/config/sestatus.h) \
    $(wildcard include/config/unicode/support.h) \
    $(wildcard include/config/feature/mtab/support.h) \
    $(wildcard include/config/feature/clean/up.h) \
    $(wildcard include/config/feature/devfs.h) \
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
  /usr/include/newlib/ctype.h \
  /usr/include/newlib/_ansi.h \
  /usr/include/newlib/sys/_locale.h \
  /usr/include/newlib/dirent.h \
  /home/jncronin/src/gk/include/sys/dirent.h \
  /home/jncronin/src/gk/include/bits/dirent.h \
  /usr/include/newlib/sys/lock.h \
  /usr/include/newlib/errno.h \
  /usr/include/newlib/sys/errno.h \
  /usr/include/newlib/sys/reent.h \
  /usr/include/newlib/assert.h \
  /usr/include/newlib/fcntl.h \
  /usr/include/newlib/sys/fcntl.h \
  /usr/include/newlib/sys/_default_fcntl.h \
  /usr/include/newlib/sys/stat.h \
  /usr/include/newlib/time.h \
  /usr/include/newlib/machine/time.h \
  /usr/include/newlib/sys/time.h \
  /usr/include/newlib/machine/_time.h \
  /usr/include/newlib/inttypes.h \
  /usr/include/newlib/sys/_intsup.h \
  /home/jncronin/src/gk/include/netdb.h \
  /usr/include/newlib/machine/ansi.h \
  /usr/include/newlib/stdio.h \
  /usr/lib/gcc/arm-none-eabi/10.3.1/include/stdarg.h \
  /usr/include/newlib/sys/stdio.h \
  /usr/include/newlib/setjmp.h \
  /usr/include/newlib/machine/setjmp.h \
  /usr/include/newlib/signal.h \
  /usr/include/newlib/sys/signal.h \
  /usr/include/newlib/paths.h \
  /usr/include/newlib/stdlib.h \
  /usr/include/newlib/machine/stdlib.h \
  /usr/include/newlib/alloca.h \
  /usr/include/newlib/string.h \
  /usr/include/newlib/strings.h \
  /usr/include/newlib/sys/string.h \
  /usr/include/newlib/libgen.h \
  /home/jncronin/src/gk/include/poll.h \
  /home/jncronin/src/gk/include/sys/ioctl.h \
  /home/jncronin/src/gk/include/sys/mman.h \
  /home/jncronin/src/gk/include/sys/resource.h \
  /home/jncronin/src/gk/include/sys/socket.h \
  /home/jncronin/src/gk/include/sys/uio.h \
  /home/jncronin/src/gk/include/sys/sysmacros.h \
  /usr/include/newlib/sys/wait.h \
  /usr/include/newlib/termios.h \
  /home/jncronin/src/gk/include/sys/termios.h \
  /usr/include/newlib/sys/param.h \
  /usr/include/newlib/sys/syslimits.h \
  /usr/include/newlib/machine/param.h \
  /usr/include/newlib/pwd.h \
  /usr/include/newlib/grp.h \
  /home/jncronin/src/gk/include/mntent.h \
  /home/jncronin/src/gk/include/sys/statfs.h \
  /home/jncronin/src/gk/include/arpa/inet.h \
  include/xatonum.h \

libbb/fgets_str.o: $(deps_libbb/fgets_str.o)

$(deps_libbb/fgets_str.o):
