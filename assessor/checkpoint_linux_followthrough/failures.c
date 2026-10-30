#define _GNU_SOURCE
#include "bridge.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "P5_FAILURE_CHECK line=%d %s\n", __LINE__, #x); \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

enum failure_limit
{
    WAIT_LIMIT_SECONDS = 3
};

/*
 * Fault seams built with the linker's --wrap: the next pthread_create can be
 * made to fail with EAGAIN, and the close of one chosen descriptor can be made
 * to report a late error after the descriptor was really released.
 */
int __real_pthread_create(pthread_t *, const pthread_attr_t *,
                          void *(*)(void *), void *);
int __real_close(int);
static int fail_create;
static int fail_close = -1; /* Descriptor whose close reports an error. */
static int close_error = EIO;
static int replacement = -1; /* Descriptor that reused the closed number. */
static unsigned targeted_calls;

int __wrap_pthread_create(pthread_t *t, const pthread_attr_t *a,
                          void *(*f)(void *), void *p)
{
    if (fail_create)
    {
        fail_create = 0;
        return EAGAIN;
    }
    return __real_pthread_create(t, a, f, p);
}

int __wrap_close(int fd)
{
    if (fd != fail_close)
    {
        return __real_close(fd);
    }
    ++targeted_calls;
    CHECK(!__real_close(fd));
    /* Real descriptor retirement/reuse, injected Linux late-error outcome.
       No disk error or signal delivery is claimed by this deterministic seam. */
    replacement = fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, fd);
    CHECK(replacement == fd);
    errno = close_error;
    return -1;
}

/*
 * A failed pthread_create must roll back to a state where begin can be
 * repeated. A late close error on a descriptor number that another owner
 * immediately reuses must be reported once, recorded, never retried, and must
 * leave the reused descriptor open.
 */
static void check_creation_rollback_and_late_close(int error)
{
    int p[2];
    CHECK(!pipe2(p, O_NONBLOCK | O_CLOEXEC));
    struct p5_bridge b;
    CHECK(!p5_bridge_init(&b, p[0], 0));
    CHECK(!p5_bridge_offer(&b, 1));
    uint32_t g = 99;
    fail_create = 1;
    CHECK(p5_bridge_begin(&b, &g) == -1 && g == 99 && !b.started &&
          b.operation.state == F_DONE && b.operation.releases == 1 &&
          b.queue.count == 1);
    CHECK(!p5_bridge_begin(&b, &g) && g == 2);
    CHECK(p5_bridge_cancel(&b, g) == 1);
    struct timespec deadline;
    CHECK(!clock_gettime(CLOCK_MONOTONIC, &deadline));
    deadline.tv_sec += WAIT_LIMIT_SECONDS;
    CHECK(!p5_bridge_wait(&b, &deadline) && !p5_bridge_observe(&b, g) &&
          !p5_bridge_reap(&b));
    CHECK(b.operation.releases == 2 && b.discarded == 1 && b.joins == 1);
    fail_close = b.fd;
    close_error = error;
    targeted_calls = 0;
    CHECK(p5_bridge_destroy(&b) == -1 && b.cleanup_error == error &&
          !b.initialized && b.fd == -1 && targeted_calls == 1);
    CHECK(replacement == fail_close && fcntl(replacement, F_GETFD) >= 0);
    fail_close = -1;
    CHECK(!close(replacement) && !close(p[0]) && !close(p[1]));
}

/*
 * Destroying a bridge whose worker could not be created abandons the queued
 * items without retrying, and closes the descriptors it owns.
 */
static void check_abandon_after_failed_create(void)
{
    int p[2];
    CHECK(!pipe2(p, O_NONBLOCK | O_CLOEXEC));
    struct p5_bridge b;
    CHECK(!p5_bridge_init(&b, p[0], 0));
    CHECK(!p5_bridge_offer(&b, 1) && !p5_bridge_offer(&b, 2));
    fail_create = 1;
    uint32_t g = 77;
    CHECK(p5_bridge_begin(&b, &g) == -1 && g == 77 && b.queue.count == 2);
    CHECK(!p5_bridge_destroy(&b) && b.discarded == 2 &&
          b.operation.releases == 1 && !b.initialized && !close(p[0]) &&
          !close(p[1]));
}

int main(void)
{
    check_creation_rollback_and_late_close(EIO);
    check_creation_rollback_and_late_close(EINTR);
    check_abandon_after_failed_create();
    puts(
        "P5_FAILURE_PASS creation_rollback=3 late_close_errors=2 reused_fd_preserved=2 no_close_retry abandon_without_retry");
    return 0;
}
