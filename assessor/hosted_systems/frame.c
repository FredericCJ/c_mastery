#include "common.h"

int hs_read_frame(int fd, void *buffer, size_t cap, size_t *length)
{
    if (!length || (!buffer && cap))
        return -1;
    unsigned char head[2];
    int r = read_exact(fd, head, 2);
    if (r)
        return r;
    size_t n = ((size_t)head[0] << 8) | head[1];
    if (n > cap)
        return -3;
    r = read_exact(fd, buffer, n);
    if (!r)
        *length = n;
    return r;
}
