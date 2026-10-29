#include "common.h"

int hs_shared_roundtrip(unsigned char *m, size_t cap, unsigned off,
                        const char *text, size_t n)
{
    if (!m || (!text && n) || off < 8 || off > cap || n > cap - off ||
        n > UINT32_MAX)
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
        uint32_t length = (uint32_t)n, offset = off;
        memcpy(m, &offset, 4);
        memcpy(m + 4, &length, 4);
        if (n)
        {
            memcpy(m + off, text, n);
        }
        _exit(0);
    }
    int status;
    return reap(p, &status) || !WIFEXITED(status) || WEXITSTATUS(status) ? -1
                                                                         : 0;
}
