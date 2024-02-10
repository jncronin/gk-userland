#ifndef POLL_H
#define POLL_H

struct pollfd
{
    int fd;
    short int events;
    short int revents;
};

typedef unsigned int nfds_t;

#define POLLRDNORM      1
#define POLLRDBAND      2
#define POLLIN          3
#define POLLPRI         4
#define POLLOUT         8
#define POLLWRNORM      POLLOUT
#define POLLWRBAND      16
#define POLLERR         32
#define POLLHUP         64
#define POLLNVAL        128

int poll(struct pollfd[], nfds_t, int);

#endif
