/*
 * Public bridge test: a real nonblocking pipe, a gated worker thread and the
 * imported struct f_async / in_shutdown composition.
 *
 * Phases run in order against one bridge; each depends on the state the
 * previous one left. A failed CHECK prints "P5_CHECK line=N expression" on
 * stderr and exits 1; success prints P5_BRIDGE_PASS.
 */
#define _GNU_SOURCE
#include "bridge.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "P5_CHECK line=%d %s\n", __LINE__, #x); \
            exit(1);                                                \
        }                                                           \
    } while (0)

enum bridge_test_limit
{
    PIPE_PAYLOAD_BYTES = 3,
    WAIT_LIMIT_SECONDS = 3,
    UNSET_TICKET = 99
};

struct bridge_fixture
{
    int fds[2];
    struct p5_bridge bridge;
    uint32_t ticket;
};

/* Returns the CLOCK_MONOTONIC time `seconds` from now. */
static struct timespec deadline_in(unsigned seconds)
{
    struct timespec t;
    CHECK(!clock_gettime(CLOCK_MONOTONIC, &t));
    t.tv_sec += seconds;
    return t;
}

/* Blocks (bounded) until the worker has read `expected` bytes of "ABC". */
static void read_progress(struct p5_bridge *p, size_t expected)
{
    struct timespec limit = deadline_in(WAIT_LIMIT_SECONDS);
    CHECK(!pthread_mutex_lock(&p->mutex));
    while (p->used < expected)
    {
        CHECK(!pthread_cond_timedwait(&p->changed, &p->mutex, &limit));
    }
    CHECK(!memcmp(p->bytes, "ABC", expected));
    CHECK(!pthread_mutex_unlock(&p->mutex));
}

/* Starts the first worker with two queued items and 3 bytes of input. */
static void start_first_operation(struct bridge_fixture *f)
{
    struct p5_bridge *p = &f->bridge;
    CHECK(!pipe2(f->fds, O_NONBLOCK | O_CLOEXEC));
    CHECK(!p5_bridge_init(p, f->fds[0], 1));
    CHECK(!close(f->fds[0])); /* The bridge owns its duplicate. */
    CHECK(!p5_bridge_offer(p, 1) && !p5_bridge_offer(p, 2));
    f->ticket = UNSET_TICKET;
    CHECK(!p5_bridge_begin(p, &f->ticket) && f->ticket == 1);
    CHECK(write(f->fds[1], "ABC", PIPE_PAYLOAD_BYTES) == PIPE_PAYLOAD_BYTES);
    read_progress(p, PIPE_PAYLOAD_BYTES);
}

/*
 * A cancel request is not an acknowledgement: while the gate withholds
 * quiescence nothing may be released, discarded or freed.
 */
static void check_request_is_not_ack(struct bridge_fixture *f)
{
    struct p5_bridge *p = &f->bridge;
    const uint32_t g = f->ticket;
    CHECK(p5_bridge_cancel(p, g) == 1 && p5_bridge_cancel(p, g) == 1);
    const struct timespec now = deadline_in(0);
    CHECK(p5_bridge_wait(p, &now) == 1);
    CHECK(p5_bridge_observe(p, g) == 1 && p5_bridge_reap(p) == 1 &&
          p5_bridge_destroy(p) == 1);
    CHECK(!pthread_mutex_lock(&p->mutex));
    CHECK(p->queue.count == 2 && p->operation.releases == 0 &&
          p->discarded == 0 && p->callbacks == 0 && !p->quiescent);
    CHECK(!pthread_mutex_unlock(&p->mutex));
}

/* Releasing the gate gives quiescence; only then are items discarded. */
static void check_ack_then_join(struct bridge_fixture *f)
{
    struct p5_bridge *p = &f->bridge;
    const uint32_t g = f->ticket;
    CHECK(!p5_bridge_release_gate(p));
    const struct timespec limit = deadline_in(WAIT_LIMIT_SECONDS);
    CHECK(!p5_bridge_wait(p, &limit));
    CHECK(!p5_bridge_observe(p, g) && !p5_bridge_observe(p, g));
    CHECK(p->operation.releases == 1 && p->discarded == 2 &&
          p->queue.count == 0 && p->callbacks == 1 && p->joins == 0);
    CHECK(p5_bridge_destroy(p) == 1 && fcntl(p->fd, F_GETFD) >= 0);
    CHECK(!p5_bridge_reap(p) && p->joins == 1);
}

/*
 * A second operation gets a fresh ticket; the old ticket is stale for every
 * call and must not release the new operation.
 */
static void check_stale_ticket_refused(struct bridge_fixture *f)
{
    struct p5_bridge *p = &f->bridge;
    const uint32_t stale = f->ticket;
    CHECK(!p5_bridge_offer(p, 3));
    uint32_t next = UNSET_TICKET;
    CHECK(!p5_bridge_begin(p, &next) && next == 2);
    CHECK(p5_bridge_cancel(p, stale) == -1 && p5_bridge_cancel(p, next) == 1);
    CHECK(p5_bridge_observe(p, stale) == -1 && p->operation.releases == 1);
    CHECK(!p5_bridge_release_gate(p));
    const struct timespec limit = deadline_in(WAIT_LIMIT_SECONDS);
    CHECK(!p5_bridge_wait(p, &limit));
    CHECK(!p5_bridge_observe(p, next) && p->operation.releases == 2 &&
          p->discarded == 3 && p->callbacks == 2);
    CHECK(!p5_bridge_reap(p) && p->joins == 2);
    f->ticket = next;
}

/*
 * When every generation is used up, begin must refuse rather than wrap onto an
 * old ticket; destroy then closes the owned descriptors exactly once.
 */
static void check_generation_exhausted(struct bridge_fixture *f)
{
    struct p5_bridge *p = &f->bridge;
    p->operation.generation = UINT32_MAX;
    uint32_t next = UNSET_TICKET;
    CHECK(p5_bridge_begin(p, &next) == -1 && next == UNSET_TICKET);
    CHECK(!p5_bridge_destroy(p) && !p->initialized && !close(f->fds[1]));
}

/* Synchronous original composition: a false completion retains both items. */
static void check_shutdown_composition(void)
{
    algo_ring q;
    struct f_async op = {F_IDLE, 0, 0};
    uint32_t ticket;
    unsigned discarded = 0;
    CHECK(algo_ring_init(&q, 4) == ALGO_OK && algo_ring_put(&q, 1) == ALGO_OK &&
          algo_ring_put(&q, 2) == ALGO_OK && f_submit(&op, &ticket) == F_OK);
    CHECK(in_shutdown(&q, &op, 0, ticket, &discarded) == 1 && q.count == 2 &&
          !op.releases);
    CHECK(in_shutdown(&q, &op, 1, ticket, &discarded) == 0 && q.count == 0 &&
          discarded == 2 && op.releases == 1);
}

int main(void)
{
    struct bridge_fixture fixture;
    start_first_operation(&fixture);
    check_request_is_not_ack(&fixture);
    check_ack_then_join(&fixture);
    check_stale_ticket_refused(&fixture);
    check_generation_exhausted(&fixture);
    check_shutdown_composition();
    puts(
        "P5_BRIDGE_PASS real_pipe=3 callbacks=2 joins=2 retained_deadline=1 exact_FAsync_import");
    return 0;
}
