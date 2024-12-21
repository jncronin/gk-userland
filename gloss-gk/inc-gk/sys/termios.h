#ifndef _SYS_TERMIOS_H
#define _SYS_TERMIOS_H

typedef unsigned int cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS        32

struct termios
{
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t     c_cc[NCCS];
};

speed_t cfgetispeed(const struct termios *);
speed_t cfgetospeed(const struct termios *);
int     cfsetispeed(struct termios *, speed_t);
int     cfsetospeed(struct termios *, speed_t);
int     tcdrain(int);
int     tcflow(int, int);
int     tcflush(int, int);
int     tcgetattr(int, struct termios *);
int     tcsendbreak(int, int);
int     tcsetattr(int, int, const struct termios *);

#define TCIFLUSH        1
#define TCOFLUSH        2
#define TCIOFLUSH       3

/* c_lflag constants */
#define ECHO            1
#define ECHOE           2
#define ECHOK           4
#define ECHONL          8
#define ICANON          16
#define ISIG            32
#define IEXTEN          64
#define NOFLSH          128
#define TOSTOP          256

/* c_iflag constants */
#define IXON            1
#define IXOFF           2
#define ICRNL           4
#define IXANY           8
#define BRKINT          16
#define IGNBRK          32
#define IGNCR           64
#define IGNPAR          128
#define INLCR           256
#define INPCK           512
#define ISTRIP          1024
#define PARMRK          2048

/* c_oflag constants */
#define OPOST           1
#define ONLCR           2
#define OCRNL           4
#define ONOCR           8
#define ONLRET          16
#define OFILL           32
#define NLDLY           64
#define CRDLY           128
#define TABDLY          256
#define BSDLY           512
#define VTDLY           1024
#define FFDLY           2048

/* c_cflag constants */
#define CBAUD           0x1f
#define CBAUDEX         0x10
#define CSIZE           0x1e0
#define CS5             0x20
#define CS6             0x40
#define CS7             0x80
#define CS8             0x100
#define CSTOPB          0x200
#define CREAD           0x400
#define PARENB          0x800
#define PARODD          0x1000
#define HUPCL           0x2000
#define CLOCAL          0x4000
#define CIBAUD          0x8000
#define CMSPAR          0x10000
#define CRTSCTS         0x20000

#define TCSANOW         1
#define TCSADRAIN       2
#define TCSAFLUSH       3

#define VINTR           8
#define VEOF            9
#define VMIN            10
#define VTIME           11
#define VSTART          12
#define VSTOP           13
#define VQUIT           14
#define VERASE          15
#define VKILL           16
#define VEOL            17
#define VSUSP           18

#define B0              0
#define B50             1
#define B75             2
#define B110            3
#define B134            4
#define B150            5
#define B200            6
#define B300            7
#define B600            8
#define B1200           9
#define B1800           10
#define B2400           11
#define B4800           12
#define B9600           13
#define B19200          14
#define B38400          15
#define B57600          16
#define B115200         17
#define B230400         18
#define B460800         19
#define B500000         20
#define B576000         21
#define B921600         22
#define B1000000        23
#define B1152000        24
#define B1500000        25
#define B2000000        26

/* ioctls - random prefix to hopefully avoid clashes */
#define TIOCMGET        0x17890000
#define TIOCMSET        0x17890001
#define TIOCMBIC        0x17890002
#define TIOCMBIS        0x17890003
#define TIOCSBRK        0x17890004
#define TIOCCBRK        0x17890005

/* flags for above */
#define TIOCM_LE        0x1
#define TIOCM_DTR       0x2
#define TIOCM_RTS       0x4
#define TIOCM_ST        0x8
#define TIOCM_SR        0x10
#define TIOCM_CTS       0x20
#define TIOCM_CAR       0x40
#define TIOCM_CD        TIOCM_CAR
#define TIOCM_RNG       0x80
#define TIOCM_RI        TIOCM_RNG
#define TIOCM_DSR       0x100

#endif
