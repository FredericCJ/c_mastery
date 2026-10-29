#define _POSIX_C_SOURCE 200809L
#include "linux.h"
#include "test.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
ssize_t __real_pwrite(int, const void *, size_t, off_t);
int __real_fsync(int);
static int stop = -1, committed, interrupt_once, fail_sync;

ssize_t __wrap_pwrite(int fd, const void *p, size_t n, off_t at)
{
    if (interrupt_once)
    {
        interrupt_once = 0;
        errno = EINTR;
        return -1;
    }
    if (stop >= 0 && committed == stop)
    {
        errno = EIO;
        return -1;
    }
    if (n > 1)
        n = 1;
    ssize_t result = __real_pwrite(fd, p, n, at);
    if (result > 0)
        committed += (int)result;
    return result;
}

int __wrap_fsync(int fd)
{
    if (fail_sync)
    {
        errno = EIO;
        return -1;
    }
    return __real_fsync(fd);
}

int main(void)
{
    char name[] = "/tmp/cmastery-outbox-fault-XXXXXX";
    int fd = mkstemp(name);
    CHECK(fd >= 0 && unlink(name) == 0);
    for (int cut = 0; cut < 12; ++cut)
    {
        CHECK(ftruncate(fd, 0) == 0);
        stop = cut;
        committed = 0;
        interrupt_once = 1;
        CHECK(ar_outbox(fd, 1, 6, 0) == AR_FAILED && committed == cut);
        stop = -1;
        int result = ar_outbox(fd, 1, 6, 0);
        CHECK(result == (cut ? AR_FAILED : AR_OK));
    }
    CHECK(ftruncate(fd, 0) == 0);
    committed = 0;
    fail_sync = 1;
    CHECK(ar_outbox(fd, 1, 6, 0) == AR_FAILED && committed == 12);
    CHECK(ar_outbox(fd, 1, 6, 0) == AR_FAILED);
    fail_sync = 0;
    CHECK(ar_outbox(fd, 1, 6, 0) == AR_OK);
    CHECK(fcntl(fd, F_SETFL, O_APPEND) == 0 &&
          ar_outbox(fd, 1, 6, 0) == AR_INVALID);
    CHECK(close(fd) == 0);
    puts("PASS outbox actual short writes,EINTR,every torn prefix,failed "
         "sync,retry sync,append rejection");
}
