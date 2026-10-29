#define _GNU_SOURCE
#include "durable.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int du_replace(int a, int b, const unsigned char *p, size_t n, unsigned fail,
               unsigned *phase, du_hook_fn hook, void *context)
{
    int fd = -1, err = 0;
    *phase = 0;
    if (n > 4096 || fail > 5)
    {
        return -EINVAL;
    }
    for (unsigned step = 1; step <= 5; step++)
    {
        if (fail == step)
        {
            err = EIO;
            break;
        }
        if (step == 1)
        {
            fd =
                openat(a, "tmp", O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC, 0600);
            if (fd < 0)
            {
                err = errno;
                break;
            }
            size_t done = 0;
            while (done < n)
            {
                ssize_t k = write(fd, p + done, n - done);
                if (k < 0 && errno == EINTR)
                {
                    continue;
                }
                if (k <= 0)
                {
                    err = k < 0 ? errno : EIO;
                    break;
                }
                done += (size_t)k;
            }
            if (err)
            {
                break;
            }
        }
        else if (step == 2)
        {
            if (fsync(fd) < 0)
            {
                err = errno;
                break;
            }
        }
        else if (step == 3)
        {
            if (renameat(a, "tmp", b, "target") < 0)
            {
                err = errno;
                break;
            }
        }
        else if (step == 4)
        {
            if (fsync(a) < 0)
            {
                err = errno;
                break;
            }
        }
        else
        {
#ifndef DU_NO_DEST_SYNC
            if (fsync(b) < 0)
            {
                err = errno;
                break;
            }
#endif
        }
        *phase = step;
        if (hook)
        {
            hook(step, context);
        }
    }
    if (fd >= 0 && close(fd) < 0 && !err)
    {
        err = errno;
    }
    return err ? -err : 0;
}
