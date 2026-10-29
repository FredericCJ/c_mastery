#include "architecture.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "TA_SOCKET_CHECK:%d:%s errno=%d\n", __LINE__, #x, \
                    errno);                                                   \
            return 1;                                                         \
        }                                                                     \
    } while (0)

struct ta_socket
{
    int fd, error;
    size_t calls, accepted;
};

static struct ta_write_result send_one(void *p, const unsigned char *bytes,
                                       size_t n)
{
    struct ta_socket *s = p;
    ssize_t count;
    s->calls++;
    /* Intentional adapter cap, not a claim the kernel shortened a one-byte call. */
    count = send(s->fd, bytes, n > 1 ? 1 : n, MSG_NOSIGNAL);
    if (count < 0)
    {
        s->error = errno;
        return (struct ta_write_result){TA_WRITE_ERROR, 0};
    }
    s->accepted += (size_t)count;
    return (struct ta_write_result){TA_WRITE_OK, (size_t)count};
}

int main(void)
{
    int fds[2];
    unsigned char packet[3] = {2, 7, 8}, got[2];
    struct ta_flow f;
    struct ta_socket s;
    CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);
    s = (struct ta_socket){fds[0], 0, 0, 0};
    CHECK(ta_begin(&f, packet, 3, TA_RING, 3) == TA_OK);
    CHECK(ta_step(&f, send_one, &s) == TA_AGAIN);
    CHECK(ta_step(&f, send_one, &s) == TA_DONE);
    CHECK(recv(fds[1], got, 2, MSG_WAITALL) == 2 && got[0] == 7 && got[1] == 8);
    CHECK(s.calls == 2 && s.accepted == 2);
    CHECK(close(fds[1]) == 0);
    CHECK(ta_begin(&f, packet, 3, TA_LINKED, 3) == TA_OK);
    CHECK(ta_step(&f, send_one, &s) == TA_IO && s.error == EPIPE);
    CHECK(close(fds[0]) == 0);
    /* Separate real nonblocking kernel boundary, independent of scripted cap. */
    CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);
    int capacity = 4096;
    CHECK(setsockopt(fds[0], SOL_SOCKET, SO_SNDBUF, &capacity,
                     sizeof capacity) == 0);
    unsigned char *bytes = malloc(65536);
    CHECK(bytes);
    memset(bytes, 0x5a, 65536);
    size_t total = 0;
    unsigned calls = 0;
    int partial = 0, blocked = 0;
    for (; calls < 64; calls++)
    {
        ssize_t n = send(fds[0], bytes, 65536, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (n < 0)
        {
            CHECK(errno == EAGAIN || errno == EWOULDBLOCK);
            blocked = 1;
            break;
        }
        CHECK(n > 0);
        partial |= n < 65536;
        total += (size_t)n;
    }
    CHECK(partial && blocked && total > 0);
    CHECK(shutdown(fds[0], SHUT_WR) == 0);
    size_t received = 0;
    for (;;)
    {
        ssize_t n = recv(fds[1], bytes, 65536, 0);
        CHECK(n >= 0);
        if (!n)
        {
            break;
        }
        for (ssize_t i = 0; i < n; i++)
        {
            CHECK(bytes[i] == 0x5a);
        }
        received += (size_t)n;
    }
    CHECK(received == total);
    free(bytes);
    CHECK(close(fds[0]) == 0);
    CHECK(close(fds[1]) == 0);
    printf(
        "TA_SOCKET_PASS:acquire,controlled-prefix,EPIPE,close,actual-nonblocking-prefix=%zu,calls=%u\n",
        total, calls + 1);
    return 0;
}
