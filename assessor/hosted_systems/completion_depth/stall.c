#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                  \
    do                                                            \
    {                                                             \
        if (!(x))                                                 \
        {                                                         \
            fprintf(stderr, "STALL-CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                              \
        }                                                         \
    } while (0)

static uint64_t now(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (uint64_t)t.tv_sec * 1000000000u + (unsigned long)t.tv_nsec;
}

static void one(int copy)
{
    char path[] = "/tmp/cmastery-stall-XXXXXX";
    int fd = mkstemp(path);
    CHECK(fd >= 0 && unlink(path) == 0);
    unsigned char b[4096];
    for (unsigned i = 0; i < sizeof b; ++i)
    {
        b[i] = (unsigned char)(i % 251);
    }
    for (unsigned i = 0; i < 256; ++i)
    {
        CHECK(write(fd, b, sizeof b) == sizeof b);
    }
    int s[2], size = 1024;
    CHECK(socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                     s) == 0 &&
          setsockopt(s[0], SOL_SOCKET, SO_SNDBUF, &size, sizeof size) == 0);
    size_t sent = 0, received = 0;
    unsigned partial = 0, retry = 0;
    while (sent < 1048576)
    {
        size_t want = copy ? sizeof b : 1048576 - sent;
        ssize_t n;
        if (copy)
        {
            CHECK(pread(fd, b, want, (off_t)sent) == (ssize_t)want);
            n = write(s[0], b, want);
        }
        else
        {
            off_t off = (off_t)sent;
            n = sendfile(s[0], fd, &off, want);
            if (n >= 0)
            {
                CHECK(off == (off_t)(sent + (size_t)n));
            }
        }
        if (n > 0)
        {
            if ((size_t)n < want)
            {
                ++partial;
            }
            sent += (size_t)n;
        }
        else
        {
            CHECK(n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));
            ++retry;
            break;
        }
    }
    CHECK(retry == 1 && sent > 0 && sent < 1048576);
    struct timespec stall = {0, 5000000};
    CHECK(nanosleep(&stall, NULL) == 0);
    /* Stop admission after the observed full socket. No write is in flight in
     * this synchronous nonblocking backend. Half-close retains already queued
     * bytes; the source file is immutable until their actual consumption. */
    uint64_t cancel = now();
    CHECK(shutdown(s[0], SHUT_WR) == 0);
    for (;;)
    {
#ifdef CO_STALL_EARLY
        break;
#endif
        ssize_t n = read(s[1], b, sizeof b);
        if (n == 0)
        {
            break;
        }
        CHECK(n > 0);
        for (ssize_t i = 0; i < n; ++i)
        {
            CHECK(b[i] ==
                  (unsigned char)(((received + (size_t)i) % 4096) % 251));
        }
        received += (size_t)n;
    }
    CHECK(received == sent && close(s[0]) == 0 && close(s[1]) == 0 &&
          close(fd) == 0);
    printf(
        "STALL-PASS:backend=%s;sent=%zu;consumed=%zu;would_block=%u;partial=%"
        "u;cancel_to_drained_ns=%" PRIu64
        ";source_unchanged_until_drained=1;\n",
        copy ? "copy" : "sendfile", sent, received, retry, partial,
        now() - cancel);
}

int main(void)
{
    one(1);
    one(0);
}
