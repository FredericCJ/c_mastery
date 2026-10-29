#include "common.h"

int hs_copy(int in, int out, size_t lim, size_t *moved)
{
    if (!moved)
    {
        return -1;
    }
    *moved = 0;
    unsigned char buf[127];
    while (*moved < lim)
    {
        size_t n = lim - *moved;
        if (n > sizeof buf)
        {
            n = sizeof buf;
        }
        ssize_t r = retry_read(in, buf, n);
        if (r < 0)
        {
            return -1;
        }
        if (r == 0)
        {
            return 0;
        }
        size_t done = 0;
        int result = write_all(out, buf, (size_t)r, &done);
        *moved += done;
        if (result)
        {
            return -1;
        }
    }
    return 0;
}
