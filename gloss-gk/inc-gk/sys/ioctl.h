#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

int ioctl(int fd, unsigned long request, ...);
	
struct winsize
{
    unsigned short ws_row;	/* rows, in characters */
    unsigned short ws_col;	/* columns, in characters */
    unsigned short ws_xpixel;	/* horizontal size, pixels */
    unsigned short ws_ypixel;	/* vertical size, pixels */
};

#define TIOCGWINSZ  1
#define TIOCSCTTY   2

#endif
