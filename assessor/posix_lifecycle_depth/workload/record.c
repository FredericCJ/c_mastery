#define _POSIX_C_SOURCE 200809L
#include "record.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#ifndef PW_MUTANT
#define PW_MUTANT 0
#endif

enum
{
    PW_RECORD_MAX_BYTES = 64,
    PW_LOGICAL_MAX_BYTES = 1024 * 1024
};

/* Contract: reads `length` bytes at byte `offset` of the logical window
   [0, logical) of the file, as a positioned read that leaves the
   descriptor's file offset alone. The descriptor is borrowed.

   The record is first assembled in a private buffer and copied to `out`
   only when it is complete, so `out` is untouched on every failure: a
   caller never sees a partial record. Returns 0, EINVAL (bad arguments or a
   record larger than PW_RECORD_MAX_BYTES), ERANGE (outside the window or
   larger than `capacity`), ENODATA (the file is shorter than the window
   promised) or the errno of a failed pread(). Blocks as pread() does. */
int pw_read_record(int fd, size_t logical, size_t offset, size_t length,
                   void *out, size_t capacity)
{
    unsigned char staging[PW_RECORD_MAX_BYTES] = {0};
    if (logical > PW_LOGICAL_MAX_BYTES || length > sizeof staging ||
        (!out && length))
    {
        return EINVAL;
    }
    size_t limit = logical;
#if PW_MUTANT == 1
    limit += PW_RECORD_MAX_BYTES;
#endif
    if (offset > limit || length > limit - offset || length > capacity)
    {
        return ERANGE;
    }
    size_t done = 0;
    while (done < length)
    {
        ssize_t n =
            pread(fd, staging + done, length - done, (off_t)(offset + done));
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
#if PW_MUTANT == 3
            if (done)
            {
                memcpy(out, staging, done);
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
        memcpy(out, staging, length);
    }
    return 0;
}
