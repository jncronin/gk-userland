#if __GAMEKID__ == 4

#include "deferred.h"
#include "syscalls.h"
#include <stdlib.h>

extern char **environ;

struct argret
{
    int argc;
    char **argv;
};

extern "C" argret __gk_init_args()
{
    while(true)
    {
        /* ask the kernel how big the argc array is, then, because we cannot
            access kernel memory, ask the kernel to copy the array to our newly allocated memory.

            Loop in case we don't have enough space
        */
        auto env_count = deferred_call(__syscall_get_arg_count, 0);
        if(env_count < 0)
        {
            return { 0, nullptr };
        }

        auto nenvs = (char **)malloc((env_count + 1) * sizeof(char *));
        if(!nenvs)
        {
            return { 0, nullptr };
        }

        bool success = true;
        for(int i = 0; i < env_count; i++)
        {
            auto ienv_len = deferred_call(__syscall_get_iarg_size, i);
            if(ienv_len < 0)
            {
                return { 0, nullptr };
            }

            auto nenv = (char *)malloc(ienv_len + 1);
            if(!nenv)
            {
                for(int j = 0; j < i; j++)
                    free(nenvs[j]);
                free(nenvs);
                return { 0, nullptr };
            }

            __syscall_get_ienv_params p { .outbuf = nenv, .outbuf_size = (size_t)(ienv_len + 1), .i = (unsigned int)i };
            if(deferred_call(__syscall_get_iarg, &p) != 0)
            {
                // try again - something has changed the array
                success = false;
                free(nenv);

                for(int j = 0; j < i; j++)
                    free(nenvs[j]);
                break;
            }
            else
            {
                // success
                nenv[ienv_len] = 0;
                nenvs[i] = nenv;
            }
        }

        if(success)
        {
            nenvs[env_count] = nullptr;
            return { env_count, nenvs };
        }

        // else try again
        free(nenvs);
    }
}

#endif
