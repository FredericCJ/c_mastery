#include "common.h"

int hs_spawn_exit(unsigned code, int *observed)
{
    if (code > 255 || !observed)
    {
        return -1;
    }
    pid_t p = fork();
    if (p < 0)
    {
        return -1;
    }
    if (p == 0)
    {
        _exit((int)code);
    }
    int status;
    if (reap(p, &status) || !WIFEXITED(status))
    {
        return -1;
    }
    *observed = WEXITSTATUS(status);
    return 0;
}
