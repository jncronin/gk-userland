#include "deferred.h"
#include "syscalls.h"
#include <stdlib.h>

extern char **environ;

extern "C" void __gk_init_env()
{
    while(true)
    {
        /* ask the kernel how big the environment array is, then, because we cannot
            access kernel memory, ask the kernel to copy the array to our newly allocated memory.

            Need to use malloc here because newlib setenv may call realloc on it.

            Loop in case we don't have enough space
        */
        auto env_count = deferred_call(__syscall_get_env_count, 0);
        if(env_count < 0)
        {
            return;
        }

        auto nenvs = (char **)malloc((env_count + 1) * sizeof(char *));
        if(!nenvs)
        {
            return;
        }

        bool success = true;
        for(int i = 0; i < env_count; i++)
        {
            auto ienv_len = deferred_call(__syscall_get_ienv_size, i);
            if(ienv_len < 0)
            {
                return;
            }

            auto nenv = (char *)malloc(ienv_len + 1);
            if(!nenv)
            {
                for(int j = 0; j < i; j++)
                    free(nenvs[j]);
                free(nenvs);
                return;
            }

            __syscall_get_ienv_params p { .outbuf = nenv, .outbuf_size = (size_t)(ienv_len + 1), .i = (unsigned int)i };
            if(deferred_call(__syscall_get_ienv, &p) != 0)
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
            // null terminate array
            nenvs[env_count] = 0;

            environ = nenvs;
            return;
        }

        // else try again
        free(nenvs);
    }
}
