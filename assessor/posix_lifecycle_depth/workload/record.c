#define _POSIX_C_SOURCE 200809L
#include "record.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#ifndef PW_MUTANT
#define PW_MUTANT 0
#endif
int pw_read_record(int fd, size_t logical, size_t offset, size_t length,
                   void *out, size_t capacity)
{
    unsigned char temp[64] = {0};
    size_t limit = logical;
    if (logical > 1024 * 1024 || length > sizeof temp || (!out && length))
    {
        return EINVAL;
    }
#if PW_MUTANT == 1
    limit += 64;
#endif
    if (offset > limit || length > limit - offset || length > capacity)
    {
        return ERANGE;
    }
    size_t done = 0;
    while (done < length)
    {
        ssize_t n =
            pread(fd, temp + done, length - done, (off_t)(offset + done));
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
#if PW_MUTANT == 3
            if (done)
            {
                memcpy(out, temp, done);
            }
#endif
            return errno;
        }
        if (!n)
        {
            return ENODATA;
        }
#if PW_MUTANT == 2
        done = length;
#else
        done += (size_t)n;
#endif
    }
    if (length)
    {
        memcpy(out, temp, length);
    }
    return 0;
}
