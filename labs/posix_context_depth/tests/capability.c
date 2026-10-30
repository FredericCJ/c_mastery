#define _GNU_SOURCE
#include "context.h"
#include "test.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/* Backend numbers of struct pc_capability. */
enum
{
    BACKEND_EPOLL = 1,
    BACKEND_POLL = 2,
    BACKEND_UNSUPPORTED = 3
};

/* Decision model: probe results are injected, so no kernel feature has to be
 * missing for the fallback paths to run. One epoch is one decision. */
static void decision_model(void)
{
    struct pc_capability capability = {0};
    CHECK(pc_choose(&capability, 1, 0, 1, 0) == 0 &&
          capability.backend == BACKEND_EPOLL);
    /* Same epoch: the first decision stands whatever the new probe says. */
    CHECK(pc_choose(&capability, 1, ENOSYS, 1, 0) == 0 &&
          capability.backend == BACKEND_EPOLL);
    /* A new epoch while an operation runs must not change state. */
    struct pc_capability saved = capability;
    CHECK(pc_choose(&capability, 2, ENOSYS, 1, 1) == EBUSY &&
          !memcmp(&capability, &saved, sizeof capability));
    CHECK(pc_choose(&capability, 2, ENOSYS, 1, 0) == 0 &&
          capability.backend == BACKEND_POLL);
    CHECK(pc_choose(&capability, 3, EOPNOTSUPP, 1, 0) == 0 &&
          capability.backend == BACKEND_POLL);
    /* A real I/O error is preserved, not hidden behind a fallback. */
    CHECK(pc_choose(&capability, 4, EIO, 1, 0) == EIO &&
          capability.backend == BACKEND_UNSUPPORTED);
    /* No equivalent fallback: the unsupported error is reported. */
    CHECK(pc_choose(&capability, 5, ENOSYS, 0, 0) == ENOSYS &&
          capability.backend == BACKEND_UNSUPPORTED);
    CHECK(pc_choose(&capability, 6, 0, 1, 0) == 0 &&
          capability.backend == BACKEND_EPOLL);
    CHECK(pc_operation_status(ENOSYS, 0) == 1 &&
          pc_operation_status(EOPNOTSUPP, 1) == 2 &&
          pc_operation_status(0, 2) == 0);
    puts(
        "PC_CAP_MODEL epochs=6 unsupported=ENOSYS,EOPNOTSUPP io_error_preserved weaker_fallback_rejected partial_not_replayed");
}

/* Real readiness through each backend on a pipe: empty, readable twice
 * (level-triggered, no consumption), drained, then end of file. */
static void backend_readiness(int backend)
{
    int channel[2];
    char byte;
    CHECK(pipe2(channel, O_CLOEXEC) == 0);
    CHECK(pc_ready(backend, channel[0]) == 0);
    CHECK(write(channel[1], "x", 1) == 1 &&
          pc_ready(backend, channel[0]) == 1 &&
          pc_ready(backend, channel[0]) == 1);
    CHECK(read(channel[0], &byte, 1) == 1 && byte == 'x' &&
          pc_ready(backend, channel[0]) == 0);
    close(channel[1]);
    CHECK(pc_ready(backend, channel[0]) == 1);
    close(channel[0]);
    printf("PC_BACKEND %s empty=0 readable=1 repeated=1 drained=0 eof=1\n",
           backend == BACKEND_EPOLL ? "epoll" : "poll");
}

int main(void)
{
    decision_model();
    for (int backend = BACKEND_EPOLL; backend <= BACKEND_POLL; ++backend)
    {
        backend_readiness(backend);
    }
    CHECK(pc_ready(BACKEND_UNSUPPORTED, 0) == -1 && errno == ENOTSUP);
    puts("PC_CAPABILITY_PASS");
    return 0;
}
