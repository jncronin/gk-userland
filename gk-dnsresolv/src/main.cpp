/* This project is the user-side of kernel DNS resolution.
    The kernel runs this program with (perhaps many) arguments describing host(s)
    to lookup.  It then reports its results via a dedicated syscall prior to
    returning.  The kernel is waiting for it to end and then checks its
    local cache.
*/

#include <gk_resolv.hpp>
#include <arpa/inet.h>
#include <syscalls.h>
#include <cstring>

int main(int argc, char *argv[])
{
    for(auto i = 1; i < argc; i++)
    {
        const auto chost = argv[i];

        auto ret = GK_Resolv(chost, AF_UNSPEC);

        if(ret.ret == 0)
        {
            /* allocate memory for the struct to pass to the kernel */
            auto addrs = new __syscall_ipaddr[ret.addrs.size()];
            for(auto j = 0u; j < ret.addrs.size(); j++)
            {
                addrs[j].ver = ret.addrs[j]->ver;
                memset(addrs[j].addr, 0, sizeof(addrs[j].addr));

                switch(ret.addrs[j]->ver)
                {
                    case 4:
                        {
                            const auto caddr = reinterpret_cast<IP4Addr *>(ret.addrs[j].get());
                            addrs[j].addr[0] = caddr->n_addr;
                        }
                        break;

                    case 6:
                        {
                            const auto caddr = reinterpret_cast<IP6Addr *>(ret.addrs[j].get());
                            memcpy(addrs[j].addr, caddr->n_addr, sizeof(addrs[j].addr));
                        }
                        break;
                }
            }

            __syscall_adddnsentry_params p;
            p.host = chost;
            p.addrs = addrs;
            p.nentries = ret.addrs.size();

            int _errno = 0, ret = 0;
            __syscall(__syscall_adddnsentry, &ret, reinterpret_cast<void *>(&p), &_errno);

            delete[] addrs;
        }
    }

    return 0;
}
