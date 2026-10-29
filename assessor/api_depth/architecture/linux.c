#define _POSIX_C_SOURCE 200809L
#include "linux.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
struct ar_result ar_linux_read(int fd, unsigned char *p, size_t n)
{
    if (fd < 0 || !p || !n || n > 8)
        return (struct ar_result){0, AR_INVALID};
    ssize_t r = read(fd, p, n);
    if (r > 0)
        return (struct ar_result){r, AR_OK};
    if (r == 0)
        return (struct ar_result){0, AR_EMPTY};
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        return (struct ar_result){0, AR_MUTANT == 8 ? AR_EMPTY : AR_PENDING};
    return (struct ar_result){0, AR_FAILED};
}

static void put(unsigned char *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(v >> (24 - 8 * i));
}

int ar_outbox(int fd, uint32_t id, int32_t value, unsigned lose_ack)
{
    if (fd < 0 || !id || lose_ack > 1)
        return AR_INVALID;
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0 || (flags & O_APPEND) || (flags & O_ACCMODE) != O_RDWR)
        return AR_INVALID;
    unsigned char expected[12] = {0x41, 0x52, 0x30, 0x31}, old[13];
    put(expected + 4, id);
    put(expected + 8, (uint32_t)value);
    ssize_t n;
    do
    {
        n = pread(fd, old, sizeof old, 0);
    } while (n < 0 && errno == EINTR);
    if (n < 0)
        return AR_FAILED;
    if (n)
    {
        if (n != 12)
            return AR_FAILED;
        for (unsigned i = 0; i < 12; ++i)
            if (old[i] != expected[i])
                return AR_STATE;
        return fsync(fd) < 0 ? AR_FAILED : AR_OK;
    }
    size_t done = 0;
    while (done < 12)
    {
        ssize_t wrote = pwrite(fd, expected + done, 12 - done, (off_t)done);
        if (wrote < 0 && errno == EINTR)
            continue;
        if (wrote <= 0)
            return AR_FAILED;
        done += (size_t)wrote;
    }
    if (fsync(fd) < 0)
        return AR_FAILED;
    return lose_ack ? AR_PENDING : AR_OK;
}
