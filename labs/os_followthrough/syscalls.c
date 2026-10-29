#define _GNU_SOURCE
#include "raw.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "OF_SYSCALL:%d:%s:errno%d\n", __LINE__, #x, \
                    errno);                                             \
            return 1;                                                   \
        }                                                               \
    } while (0)

static ssize_t wrapped(int fd, const void *p, size_t n)
{
    long raw = of_raw_write(fd, p, n);
    if (raw < 0 && raw >= -4095)
    {
        errno = (int)-raw;
        return -1;
    }
    return raw;
}

static int short_write(unsigned native, long *sent)
{
    int p[2];
    CHECK(pipe2(p, O_NONBLOCK | O_CLOEXEC) == 0);
    int capacity = fcntl(p[1], F_GETPIPE_SZ);
    long page = sysconf(_SC_PAGESIZE);
    CHECK(capacity > 0 && capacity <= 1048576 && page > 0 && page <= capacity);
    unsigned char *bytes = malloc((size_t)capacity + (size_t)page * 2);
    CHECK(bytes);
    memset(bytes, 0x51, (size_t)capacity + (size_t)page * 2);
    CHECK(write(p[1], bytes, (size_t)capacity) == capacity);
    CHECK(read(p[0], bytes, (size_t)page) == page);
    size_t requested = (size_t)page * 2;
    long n = native ? write(p[1], bytes, requested)
                    : of_raw_write(p[1], bytes, requested);
    CHECK(n > 0 && (size_t)n < requested);
    long retry = native ? write(p[1], bytes, requested)
                        : of_raw_write(p[1], bytes, requested);
    CHECK(native ? (retry == -1 && errno == EAGAIN) : retry == -EAGAIN);
    printf("OF:short:libc%u:capacity%d:requested%zu:accepted%ld:remaining%zu:"
           "EAGAIN\n",
           native, capacity, requested, n, requested - (size_t)n);
    *sent = n;
    free(bytes);
    CHECK(close(p[0]) == 0 && close(p[1]) == 0);
    return 0;
}

int main(void)
{
    int p[2];
    CHECK(pipe2(p, O_CLOEXEC) == 0);
    const char message[] = "abc";
    char out[6];
    errno = ERANGE;
    long raw = of_raw_write(p[1], message, 3);
    int after_raw = errno;
    errno = EDOM;
    ssize_t libc = write(p[1], message, 3);
    int after_libc = errno;
    CHECK(raw == 3 && libc == 3 && read(p[0], out, 6) == 6 &&
          !memcmp(out, "abcabc", 6));
    long bad = of_raw_write(-1, message, 3);
    errno = 0;
    ssize_t converted = wrapped(-1, message, 3);
    CHECK(bad == -EBADF && converted == -1 && errno == EBADF);
    errno = 0;
    CHECK(write(-1, message, 3) == -1 && errno == EBADF);
    long fault = of_raw_write(p[1], NULL, 1);
    CHECK(fault == -EFAULT);
    printf(
        "OF:raw:success3:libc3:raw-error%ld:wrapper-1-errno%d:bad-pointer%ld:"
        "success-errno-observed%d,%d-uninterpreted\n",
        bad, EBADF, fault, after_raw, after_libc);
    CHECK(close(p[0]) == 0 && close(p[1]) == 0);
    long a, b;
    if (short_write(0, &a) || short_write(1, &b))
        return 1;
    CHECK(a == b);
    puts("OF_PASS:syscalls");
    return 0;
}
