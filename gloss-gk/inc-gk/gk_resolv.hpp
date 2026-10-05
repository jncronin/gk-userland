#ifndef RESOLV_H
#define RESOLV_H

#include <vector>
#include <cstdint>
#include <memory>

class IPAddr
{
    public:
        int ver = 0;
        virtual std::string tostring() const;
};

class IP4Addr : public IPAddr
{
    public:
        uint32_t n_addr;
        std::string tostring() const;
};

class IP6Addr : public IPAddr
{
    public:
        uint8_t n_addr[16];
};

struct ResolvResponse
{
    int ret;
    std::vector<std::unique_ptr<IPAddr>> addrs;

    ResolvResponse(const ResolvResponse &) = delete;
    ResolvResponse() = default;
    ResolvResponse(ResolvResponse &&) = default;

    ResolvResponse &operator=(ResolvResponse &&) = default;
};

ResolvResponse GK_Resolv(const char *name, int af);

#endif
