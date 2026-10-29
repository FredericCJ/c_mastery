/* Original bounded local process/message boundary. No shared pointers on wire. */
#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static int one(unsigned kind)
{
    int fds[2];
    if (pipe(fds))
    {
        return 2;
    }
    pid_t child = fork();
    if (child < 0)
    {
        return 2;
    }
    if (!child)
    {
        close(fds[0]);
        unsigned char reply[5] = {7, 9, 0, 0, 2};
        if (kind == 1)
        {
            reply[4] = 6; /* Remaining5, oversized6. */
        }
        if (kind == 2)
        {
            reply[1] = 8; /* Stale request sequence. */
        }
        size_t n = kind == 3 ? 3 : 5;
        ssize_t sent = write(fds[1], reply, n);
        close(fds[1]);
        _exit(sent == (ssize_t)n ? 0 : 2);
    }
    close(fds[1]);
    unsigned char wire[6] = {0};
    size_t got = 0;
    int infrastructure = 0;
    while (got < sizeof wire)
    {
        struct pollfd p = {fds[0], POLLIN, 0};
        if (poll(&p, 1, 1000) <= 0)
        {
            infrastructure = 1;
            break;
        }
        ssize_t n = read(fds[0], wire + got, sizeof wire - got);
        if (n < 0)
        {
            infrastructure = 1;
            break;
        }
        if (!n)
        {
            break;
        }
        got += (size_t)n;
    }
    close(fds[0]);
    int status;
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
        WEXITSTATUS(status))
    {
        return 2;
    }
    if (infrastructure)
    {
        return 2;
    }
    unsigned count =
        ((unsigned)wire[2] << 16) | ((unsigned)wire[3] << 8) | wire[4];
    int valid = got == 5 && wire[0] == 7 && wire[1] == 9 && count <= 5;
    printf("IPC_CASE:%u:bytes=%zu:accepted=%d:committed=%u\n", kind, got, valid,
           valid ? count : 0);
    return valid == (kind == 0) ? 0 : 1;
}

int main(void)
{
    for (unsigned k = 0; k < 4; k++)
    {
        int r = one(k);
        if (r)
        {
            return r;
        }
    }
    puts("IPC_PASS:extent,token,sequence,count,owned-child-reaped");
    return 0;
}
