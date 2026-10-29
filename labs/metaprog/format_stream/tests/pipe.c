#define _POSIX_C_SOURCE 200809L
#include "format_stream.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define REQUIRE(x)                                                 \
    do                                                             \
    {                                                              \
        if (!(x))                                                  \
        {                                                          \
            fprintf(stderr, "MFP_PIPE line%d %s\n", __LINE__, #x); \
            return 1;                                              \
        }                                                          \
    } while (0)

struct mp_port
{
    int fd;
    unsigned calls;
};

static struct mfp_write_result
send_prefix(void *context, const unsigned char *bytes, size_t count)
{
    struct mp_port *p = context;
    ssize_t n;
    ++p->calls;
    if (count > 2)
    {
        count = 2;
    }
    n = write(p->fd, bytes, count);
    if (n < 0)
    {
        return (struct mfp_write_result){0, (errno == EAGAIN || errno == EINTR)
                                                ? MFP_WRITE_AGAIN
                                                : MFP_WRITE_ERROR};
    }
    return (struct mfp_write_result){(size_t)n, MFP_WRITE_OK};
}

int main(void)
{
    int fds[2];
    unsigned char fill[512] = {0}, got[512];
    ssize_t n;
    struct mfp_stream stream = {0};
    struct mfp_message message = {{'h', 'e', 'l', 'l', 'o'}, 5};
    REQUIRE(pipe(fds) == 0);
    REQUIRE(fcntl(fds[0], F_SETFL, O_NONBLOCK) == 0 &&
            fcntl(fds[1], F_SETFL, O_NONBLOCK) == 0);
    struct mp_port p = {fds[1], 0};
    while ((n = write(fds[1], fill, sizeof fill)) > 0)
    {
    }
    REQUIRE(n == -1 && errno == EAGAIN);
    REQUIRE(mfp_start(&stream, &message) == MFP_OK &&
            mfp_step(&stream, send_prefix, &p) == MFP_AGAIN &&
            stream.offset == 0);
    while ((n = read(fds[0], got, sizeof got)) > 0)
    {
    }
    REQUIRE(n == -1 && errno == EAGAIN);
    REQUIRE(mfp_step(&stream, send_prefix, &p) == MFP_MORE &&
            stream.offset == 2);
    REQUIRE(mfp_step(&stream, send_prefix, &p) == MFP_MORE &&
            stream.offset == 4);
    REQUIRE(mfp_step(&stream, send_prefix, &p) == MFP_OK && stream.offset == 5);
    REQUIRE(read(fds[0], got, sizeof got) == 5 && !memcmp(got, "hello", 5));
    REQUIRE(p.calls == 4);
    REQUIRE(mfp_start(&stream, &message) == MFP_OK &&
            mfp_step(&stream, send_prefix, &p) == MFP_MORE);
    REQUIRE(mfp_cancel(&stream) == MFP_CANCELLED &&
            mfp_step(&stream, send_prefix, &p) == MFP_CANCELLED &&
            p.calls == 5);
    REQUIRE(read(fds[0], got, sizeof got) == 2 && !memcmp(got, "he", 2));
    REQUIRE(read(fds[0], got, sizeof got) == -1 && errno == EAGAIN);
    REQUIRE(close(fds[1]) == 0 && close(fds[0]) == 0);
    puts(
        "MFP_PIPE actual-nonblocking-backpressure exact-prefix cancellation-stops-output");
    return 0;
}
