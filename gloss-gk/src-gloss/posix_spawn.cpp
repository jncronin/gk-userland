#include <spawn.h>
#include <stdio.h>

extern "C" int posix_spawn(pid_t *pid,
    const char *path,
    const posix_spawn_file_actions_t *file_actions,
    const posix_spawnattr_t *attrp,
    char * const * argv,
    char * const * argp)
{
    fprintf(stderr, "posix_spawn (%s) called\n", path);
    return -1;
}

extern "C" int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions)
{
    return 0;
}

extern "C" int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions)
{
    return 0;
}

extern "C" int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *file_actions,
    int fildes,
    const char *path,
    int oflag,
    mode_t mode)
{
    return 0;
}

extern "C" int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions,
    int fildes, int newfildes)
{
    return 0;
}
