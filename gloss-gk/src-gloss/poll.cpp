#include <poll.h>
#include <stdio.h>

extern "C" int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    fprintf(stderr, "gloss-gk: poll() not implemented\n");
    return 0;
}
