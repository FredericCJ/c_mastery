#include "common.h"

ssize_t hs_local_exchange(const void *input, size_t n, void *output, size_t cap)
{
    if (n > 64 || n > cap || (!input && n) || (!output && n))
        return -1;
    int s[2];
    if (socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, s))
        return -1;
    pid_t p = fork();
    if (p < 0)
    {
        close(s[0]);
        close(s[1]);
        return -1;
    }
    if (!p)
    {
        close(s[0]);
        unsigned char b[64];
        ssize_t r = recv(s[1], b, sizeof b, 0);
        if (r < 0)
            _exit(1);
        for (ssize_t i = 0; i < r / 2; i++)
        {
            unsigned char x = b[i];
            b[i] = b[r - i - 1];
            b[r - i - 1] = x;
        }
        ssize_t sent = send(s[1], b, (size_t)r, MSG_NOSIGNAL);
        close(s[1]);
        _exit(sent == r ? 0 : 2);
    }
    close(s[1]);
    ssize_t result = -1;
    if (send(s[0], input, n, MSG_NOSIGNAL) == (ssize_t)n)
        result = recv(s[0], output, cap, 0);
    close(s[0]);
    int status;
    if (reap(p, &status) || !WIFEXITED(status) || WEXITSTATUS(status))
        return -1;
    return result;
}
