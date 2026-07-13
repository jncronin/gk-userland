#include <errno.h>
#include <cstdint>
#include <gk.h>

// TODO: support entropy source on _F chips - replace this with a syscall
static uint32_t __getentropy_int()
{
    static uint32_t centropy = 0;
    static bool has_centropy = false;

    if(!has_centropy)
    {
        centropy = (uint32_t)GK_GetCurUs();
    }

    return centropy++;
}

extern "C" int _getentropy(void *buf, size_t buflen)
{
    auto dst = (char *)buf;

    while(buflen)
    {
        auto centropy = __getentropy_int();

        *(dst++) = (char)centropy;
        centropy >>= 8;
        if(!(--buflen)) break;

        *(dst++) = (char)centropy;
        centropy >>= 8;
        if(!(--buflen)) break;

        *(dst++) = (char)centropy;
        centropy >>= 8;
        if(!(--buflen)) break;

        *(dst++) = (char)centropy;
        centropy >>= 8;
        if(!(--buflen)) break;
    }

    return 0;
}
