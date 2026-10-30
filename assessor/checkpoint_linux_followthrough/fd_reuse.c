/*
 * Descriptor-number reuse: the same fd number (TARGET_FD) is closed and
 * reopened for a new connection generation. A token captured for generation 1
 * must be rejected before any read when generation 2 holds the same number.
 *
 * Standalone: this program owns only the descriptors it creates and refuses to
 * run if TARGET_FD is already open, rather than clobbering an inherited one.
 */
#define _GNU_SOURCE
#include "checkpoint_linux.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "P5_FD_CHECK line=%d %s\n", __LINE__, #x); \
            exit(1);                                                   \
        }                                                              \
    } while (0)

enum fd_reuse_limit
{
    TARGET_FD = 7,
    QUEUE_ITEMS = 8,
    READER_SLOT = 7,
    GENERATION_LIMIT = 3,
    EPOLL_TIMEOUT_MS = 1000,
    PAYLOAD_BYTE = 42
};

/*
 * One connection generation: places a pipe's read end at TARGET_FD, sends one
 * frame, and pumps it. In generation 2, `old_token` (from generation 1) must be
 * refused as stale before touching the parser or queue. On return the
 * descriptor is closed and *old_token holds this generation's token.
 */
static void run_generation(unsigned generation, int ep, struct cpa_queue *q,
                           struct cpl_reader *r, uint64_t *old_token)
{
    int p[2];
    CHECK(!pipe2(p, O_NONBLOCK | O_CLOEXEC));
    CHECK(p[0] != TARGET_FD && p[1] != TARGET_FD);
    CHECK(fcntl(p[0], F_DUPFD_CLOEXEC, TARGET_FD) == TARGET_FD && !close(p[0]));
    CHECK(!cpl_open(r, ep, TARGET_FD, 1) && r->fd == TARGET_FD &&
          r->generation == generation);
    struct cp_message m = {.sequence = (uint16_t)generation,
                           .length = 1,
                           .payload = {PAYLOAD_BYTE}};
    unsigned char wire[CP_WIRE];
    size_t bytes;
    CHECK(cp_encode(&m, wire, sizeof wire, &bytes) == CP_OK &&
          write(p[1], wire, bytes) == (ssize_t)bytes);
    struct epoll_event event;
    CHECK(epoll_wait(ep, &event, 1, EPOLL_TIMEOUT_MS) == 1 &&
          event.data.u64 == cpl_token(r));
    if (generation == 2)
    {
        CHECK(cpl_pump(r, *old_token, q) == CPL_STALE && !q->count && !r->end &&
              !r->cursor && !r->parser.ready);
    }
    enum cpl_status s;
    do
    {
        s = cpl_pump(r, event.data.u64, q);
    } while (s == CPL_PROGRESS);
    CHECK(s == CPL_DRAINED && q->count == 1);
    struct cpa_handle h;
    struct cp_message got;
    CHECK(cpa_take(q, &h, &got, 0) == CPA_OK && got.sequence == generation &&
          cpa_finish(q, h, 0) == CPA_OK);
    *old_token = cpl_token(r);
    CHECK(!cpl_close(r) && !close(p[1]));
    CHECK(fcntl(TARGET_FD, F_GETFD) == -1 && errno == EBADF);
}

int main(void)
{
    CHECK(fcntl(TARGET_FD, F_GETFD) == -1 && errno == EBADF);
    const int ep = epoll_create1(EPOLL_CLOEXEC);
    CHECK(ep >= 0 && ep != TARGET_FD);
    struct cpa_queue q;
    struct cpl_reader r;
    CHECK(cpa_init(&q, QUEUE_ITEMS) == CPA_OK);
    cpl_init(&r, READER_SLOT, GENERATION_LIMIT);
    uint64_t old_token = 0;
    for (unsigned generation = 1; generation <= 2; generation++)
    {
        run_generation(generation, ep, &q, &r, &old_token);
    }
    CHECK(cpa_close(&q, 0) == CPA_CLOSED && cpa_destroy(&q) == CPA_OK &&
          !close(ep));
    puts("P5_FD_PASS actual_fd=7 generations=1,2 stale_rejected_before_read");
    return 0;
}
