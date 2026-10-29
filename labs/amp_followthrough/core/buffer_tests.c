#include "buffers.h"
#include <stdio.h>
#define C(x)                                                          \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "AF_BUFFER_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                                 \
        }                                                             \
    } while (0)

struct af_device
{
    struct pd_snapshot snapshot;
    unsigned log[128], n, stops;
};

static int config(void *ctx, const struct pd_extent *e, uint32_t token)
{
    struct af_device *d = ctx;
    d->snapshot = (struct pd_snapshot){token, 0, e->bytes / e->width, 0, 0};
    return 1;
}

static void enable(void *ctx)
{
    struct af_device *d = ctx;
    d->snapshot.enabled = 1;
    d->log[d->n++] = 10;
}

static void stop(void *ctx)
{
    ((struct af_device *)ctx)->stops++;
}

static struct pd_snapshot snap(void *ctx)
{
    return ((struct af_device *)ctx)->snapshot;
}

static void visible(void *ctx, unsigned op, uint32_t a, uint32_t n)
{
    struct af_device *d = ctx;
    (void)a;
    (void)n;
    d->log[d->n++] = op;
}

int main(void)
{
    struct af_device d = {0};
    struct pd_io io = {&d, config, enable, stop, snap, visible};
    struct af_buffers s = {0};
    struct xd_fence f = {0, 1, 1, 1};
    C(af_buffers_init(&s, &f) == AF_OK);
    struct pd_extent e = {0x30042000, 0x30042040, 64, 64, 64, 4};
    C(af_submit(&s, io, 0, &e) == AF_BUSY);
    C(af_fill(&s, 0) == AF_OK);
    C(af_submit(&s, io, 0, &e) == AF_OK);
    C(d.n == 5 && d.log[0] == PD_CLEAN && d.log[1] == PD_CLEAN &&
      d.log[2] == PD_INVALIDATE && d.log[3] == PD_RELEASE && d.log[4] == 10);
    C(af_fill(&s, 0) == AF_BUSY && af_release(&s, 0) == AF_BUSY &&
      af_consume(&s, 0) == AF_BUSY);
    struct pd_extent e1 = {0x30042080, 0x300420c0, 64, 64, 64, 4};
    C(af_fill(&s, 1) == AF_OK);
    C(af_submit(&s, io, 1, &e1) == AF_BUSY);
    C(af_fill(&s, 1) ==
      AF_BUSY); /* A third burst cannot overwrite either lease. */
    C(af_timeout(&s, io) == AF_BUSY && d.stops == 1);
    C(af_complete(&s, io) == AF_BUSY);
    C(af_release(&s, 0) == AF_BUSY);
    d.snapshot = (struct pd_snapshot){1, 0, 8, PD_TC, 0};
    C(af_complete(&s, io) == AF_BUSY);
    d.snapshot.fifo_empty = 1;
    C(af_complete(&s, io) == AF_OK);
    C(s.slot[0].result.terminal == XD_STOPPED &&
      s.slot[0].result.complete == 32);
    C(af_consume(&s, 0) == AF_INVALID);
    C(af_release(&s, 0) == AF_OK);
    C(af_submit(&s, io, 1, &e1) == AF_OK && s.slot[1].ticket.generation == 2);
    d.snapshot = (struct pd_snapshot){1, 0, 0, PD_TC, 1};
    C(af_complete(&s, io) == AF_BUSY); /* old generation */
    d.snapshot = (struct pd_snapshot){2, 0, 0, PD_TC | PD_TE, 1};
    C(af_complete(&s, io) == AF_OK);
    C(s.slot[1].result.terminal == XD_ERROR && s.slot[1].result.complete == 0 &&
      s.slot[1].result.possible == 64);
    C(af_consume(&s, 1) == AF_INVALID);
    C(af_release(&s, 1) == AF_OK);
    C(af_fill(&s, 0) == AF_OK && af_submit(&s, io, 0, &e) == AF_OK);
    d.snapshot = (struct pd_snapshot){3, 0, 0, PD_TC, 1};
    C(af_complete(&s, io) == AF_OK);
    C(d.log[d.n - 2] == PD_INVALIDATE && d.log[d.n - 1] == PD_ACQUIRE);
    C(af_consume(&s, 0) == AF_OK && af_release(&s, 0) == AF_OK);
    uint32_t total = 0;
    struct pd_extent changed = e;
    changed.source = 0x20000000;
    C(pd_validate(PD_H745, 1, &changed, 1, &total) == PD_INVALID);
    C(pd_validate(PD_F439, 0, &changed, 1, &total) == PD_INVALID);
    changed = (struct pd_extent){0x20000000, 0x20000100, 64, 64, 64, 4};
    C(pd_validate(PD_F439, 0, &changed, 1, &total) == PD_OK);
    changed.bytes = 4097;
    C(pd_validate(PD_F439, 0, &changed, 1, &total) == PD_INVALID);
    puts(
        "AF_PASS:buffers pingpong timeout old-token quiet-error cache-order engine-change");
    return 0;
}
