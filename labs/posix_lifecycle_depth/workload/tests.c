#define _POSIX_C_SOURCE 200809L
#include "record.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x)                                                               \
    do                                                                         \
    {                                                                          \
        if (!(x))                                                              \
        {                                                                      \
            fprintf(stderr, "PL_CHECK:%d:%s errno=%d\n", __LINE__, #x, errno); \
            exit(1);                                                           \
        }                                                                      \
    } while (0)
static unsigned calls, mode;
ssize_t __real_pread(int, void *, size_t, off_t);
ssize_t __real___pread_chk(int, void *, size_t, off_t, size_t);

static int request(size_t *n)
{
    ++calls;
    if (mode == 1)
    {
        if (calls == 1)
        {
            errno = EINTR;
            return -1;
        }
        if (*n > 3)
            *n = 3;
    }
    if (mode == 2)
    {
        if (calls == 2)
        {
            errno = EIO;
            return -1;
        }
        if (*n > 3)
            *n = 3;
    }
    return 0;
}

ssize_t __wrap_pread(int fd, void *b, size_t n, off_t offset)
{
    if (request(&n))
        return -1;
    return __real_pread(fd, b, n, offset);
}

ssize_t __wrap___pread_chk(int fd, void *b, size_t n, off_t offset,
                           size_t bound)
{
    if (request(&n))
        return -1;
    return __real___pread_chk(fd, b, n, offset, bound);
}

int main(void)
{
    char path[] = "/var/tmp/pw-record-XXXXXX";
    int fd = mkstemp(path);
    CHECK(fd >= 0 && unlink(path) == 0);
    unsigned char data[67], out[64], saved[64];
    for (size_t i = 0; i < sizeof data; ++i)
        data[i] = (unsigned char)(i + 1);
    CHECK(write(fd, data, sizeof data) == sizeof data);
    memset(out, 0xcc, sizeof out);
    memcpy(saved, out, sizeof out);
    CHECK(pw_read_record(fd, 67, 4, 32, out, sizeof out) == 0 &&
          !memcmp(out, data + 4, 32));
    CHECK(lseek(fd, 0, SEEK_CUR) == 67);
    memcpy(out, saved, sizeof out);
    CHECK(pw_read_record(fd, 67, 60, 16, out, sizeof out) == ERANGE &&
          !memcmp(out, saved, sizeof out));
    CHECK(pw_read_record(fd, 67, 68, 0, out, sizeof out) == ERANGE);
    CHECK(pw_read_record(fd, 67, 67, 0, NULL, 0) == 0);
    CHECK(pw_read_record(fd, 67, 0, 65, out, sizeof out) == EINVAL);
    CHECK(pw_read_record(fd, 67, 0, 32, out, 31) == ERANGE);
    mode = 1;
    calls = 0;
    int short_result = pw_read_record(fd, 67, 4, 32, out, sizeof out);
    printf("PW_SHORT result=%d calls=%u\n", short_result, calls);
    CHECK(short_result == 0 && calls == 12 && !memcmp(out, data + 4, 32));
    mode = 2;
    calls = 0;
    memcpy(out, saved, sizeof out);
    CHECK(pw_read_record(fd, 67, 4, 32, out, sizeof out) == EIO &&
          !memcmp(out, saved, sizeof out));
    mode = 0;
    calls = 0;
    CHECK(ftruncate(fd, 63) == 0);
    CHECK(pw_read_record(fd, 67, 60, 7, out, sizeof out) == ENODATA &&
          !memcmp(out, saved, sizeof out));
    CHECK(close(fd) == 0);
    CHECK(pw_read_record(fd, 67, 0, 1, out, sizeof out) == EBADF &&
          !memcmp(out, saved, sizeof out));
    puts(
        "PW_RECORD short_EINTR_and_EIO atomic_output logical_EOF actual_truncation");
    puts("PW_PASS");
    return 0;
}
