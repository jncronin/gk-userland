#ifndef _NET_IF_H
#define _NET_IF_H

struct if_nameindex
{
    unsigned if_index;
    char *if_name;
};

#define IF_NAMESIZE     32
unsigned              if_nametoindex(const char *);
char                 *if_indextoname(unsigned, char *);
struct if_nameindex  *if_nameindex(void);
void                  if_freenameindex(struct if_nameindex *);

#endif
