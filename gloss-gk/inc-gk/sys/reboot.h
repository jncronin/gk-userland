#ifndef _SYS_REBOOT_H
#define _SYS_REBOOT_H

int reboot(int cmd);

#define RB_HALT_SYSTEM      1
#define RB_AUTOBOOT         2
#define RB_POWERDOWN        3

#endif
