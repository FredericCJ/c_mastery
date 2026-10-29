#define _GNU_SOURCE
#include "worker.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/close_range.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

int sd_exclusive(int directory, const char *name, int *owned)
{
    size_t i;
    int fd;
    if (!name || !owned || !name[0])
        return EINVAL;
    for (i = 0; name[i]; ++i)
        if (i == 32 || name[i] == '/' || (name[i] == '.' && (i == 0)))
            return EINVAL;
    fd = openat(directory, name,
                O_CREAT |
#ifndef SD_MUTANT_EXCLUSIVE
                    O_EXCL |
#endif
                    O_RDWR | O_CLOEXEC,
                0600);
    if (fd < 0)
        return errno;
    *owned = fd;
    return 0;
}

int sd_request(unsigned allowed, enum sd_operation op, int fd, char out[4])
{
    char bytes[4];
    ssize_t n;
    if (!out || allowed > 1 || (op != SD_READ_LOG && op != SD_WRITE_CONFIG))
        return EINVAL;
#ifndef SD_MUTANT_POLICY
    if (
#ifndef SD_MUTANT_PERMISSION
        !allowed ||
#endif
        op != SD_READ_LOG)
        return EACCES;
#endif
    n = pread(fd, bytes, sizeof bytes, 0);
    if (n < 0)
        return errno;
    if (n != 4)
        return EIO;
    for (unsigned i = 0; i < 4; ++i)
        out[i] = bytes[i];
    return 0;
}

static void closed(int fd)
{
    if (fd >= 0)
        (void)close(fd);
}

int sd_spawn(const char *path, char *const argv[], char *const env[], int log,
             int temp, unsigned fault, struct sd_child *out)
{
    int stream[2] = {-1, -1}, errors[2] = {-1, -1}, l = -1, t = -1,
        result = EIO, status;
    pid_t pid;
    ssize_t n;
    int child_error = 0;
    if (!path || !argv || !env || !out || fault > 4)
        return EINVAL;
    if (fault == 1)
        return EIO;
    if (pipe2(stream, O_CLOEXEC) < 0)
        return errno;
    if (fault == 2)
        goto fail;
    if (pipe2(errors, O_CLOEXEC) < 0)
    {
        result = errno;
        goto fail;
    }
    l = fcntl(log, F_DUPFD_CLOEXEC, 10);
    if (l < 0)
    {
        result = errno;
        goto fail;
    }
    t = fcntl(temp, F_DUPFD_CLOEXEC, 10);
    if (t < 0)
    {
        result = errno;
        goto fail;
    }
    /* All sources must remain above fixture destinations0,3,4. */
    {
        int x = fcntl(stream[1], F_DUPFD_CLOEXEC, 10);
        if (x < 0)
        {
            result = errno;
            goto fail;
        }
        closed(stream[1]);
        stream[1] = x;
    }
    {
        int x = fcntl(errors[1], F_DUPFD_CLOEXEC, 10);
        if (x < 0)
        {
            result = errno;
            goto fail;
        }
        closed(errors[1]);
        errors[1] = x;
    }
    if (fault == 3)
        goto fail;
    pid = fork();
    if (pid < 0)
    {
        result = errno;
        goto fail;
    }
    if (pid == 0)
    {
        int e = EIO;
        if (dup2(stream[1], 0) < 0 || dup2(l, 4) < 0 ||
            dup3(t, 3, O_CLOEXEC) < 0)
            goto child_fail;
        closed(1);
        closed(2);
        if (syscall(SYS_close_range, 5u, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0)
            goto child_fail;
#ifdef SD_MUTANT_INHERIT
        if (fcntl(3, F_SETFD, 0) < 0)
            goto child_fail;
#endif
        if (fault == 4)
            goto child_fail;
        execve(path, argv, env);
        e = errno;
child_fail:
{
    ssize_t sent = write(errors[1], &e, sizeof e);
    _exit(sent == (ssize_t)sizeof e ? 125 : 126);
}
    }
    closed(stream[1]);
    stream[1] = -1;
    closed(errors[1]);
    errors[1] = -1;
    closed(l);
    l = -1;
    closed(t);
    t = -1;
    do
    {
        n = read(errors[0], &child_error, sizeof child_error);
    } while (n < 0 && errno == EINTR);
    closed(errors[0]);
    errors[0] = -1;
    if (n != 0)
    {
        result = n == (ssize_t)sizeof child_error ? child_error : EIO;
        while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
        {
        }
        goto fail;
    }
    out->pid = pid;
    out->output = stream[0];
    return 0;
fail:
    closed(stream[0]);
    closed(stream[1]);
    closed(errors[0]);
    closed(errors[1]);
    closed(l);
    closed(t);
    return result;
}
