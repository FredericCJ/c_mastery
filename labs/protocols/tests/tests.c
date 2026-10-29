#include "protocols.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "TS_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int typestate(void)
{
    struct pt_device d;
    struct pt_closed c, stale;
    struct pt_ready r = {0};
    pt_device_init(&d, &c);
    stale = c;
    CHECK(pt_configure(&c, 2, &r) == PT_RECOVERY &&
          d.state == PT_RECOVERY_REQUIRED && !c.device && !r.device);
    CHECK(pt_recover(&d, 0, &c) == PT_PENDING && d.control == 3);
    CHECK(pt_recover(&d, 1, &c) == PT_OK && d.cleanup == 1 && d.control == 0);
    CHECK(pt_configure(&stale, 0, &r) == PT_STATE);
    CHECK(pt_configure(&c, 0, &r) == PT_OK && d.state == PT_READY && !c.device);
    CHECK(pt_close(&r, &c) == PT_OK && !r.device && d.cleanup == 2);
    d.generation = UINT32_MAX;
    c.generation = UINT32_MAX;
    CHECK(pt_configure(&c, 0, &r) == PT_RANGE && c.device == &d);
    return 0;
}

static int builder(void)
{
    struct pt_builder b = {0};
    struct pt_config out = {91, 92, 93};
    CHECK(pt_builder_set(&b, 0, 252) == PT_OK);
    CHECK(pt_builder_finish(&b, &out) == PT_STATE && out.address == 91);
    CHECK(pt_builder_set(&b, 0, 0) == PT_STATE && b.address == 252);
    CHECK(pt_builder_set(&b, 1, 8) == PT_OK &&
          pt_builder_set(&b, 2, 10) == PT_OK);
    CHECK(pt_builder_finish(&b, &out) == PT_RANGE && out.address == 91);
    b = (struct pt_builder){0};
    CHECK(pt_builder_set(&b, 2, 0) == PT_RANGE && b.mask == 0);
    CHECK(pt_builder_set(&b, 0, 4) == 0 && pt_builder_set(&b, 1, 8) == 0 &&
          pt_builder_set(&b, 2, 10) == 0);
    CHECK(pt_builder_finish(&b, &out) == 0 && out.address == 4 &&
          out.length == 8 && out.timeout == 10);
    return 0;
}

static int session(void)
{
    for (unsigned role = 0; role < 2; role++)
    {
        for (unsigned n = 0; n <= 8; n++)
        {
            struct pt_session s = {role, 0, n};
            CHECK(pt_session_step(&s, role, PT_HELLO) == PT_STATE &&
                  s.phase == 0);
            CHECK(pt_session_step(&s, !role, PT_HELLO) == 0 && s.phase == 1);
            CHECK(pt_session_step(&s, role, PT_ACCEPT) == 0);
            for (unsigned j = 0; j < n; j++)
            {
                CHECK(pt_session_step(&s, !role, PT_DATA) == 0 && s.phase == 3);
                CHECK(pt_session_step(&s, role, PT_ACK) == 0 &&
                      s.remaining == n - j - 1);
            }
            CHECK(s.phase == 4 && pt_session_step(&s, !role, PT_BYE) == 0 &&
                  s.phase == 5);
            CHECK(pt_session_step(&s, !role, PT_BYE) == PT_STATE);
        }
    }
    return 0;
}

static int hsm(void)
{
    struct pt_machine m = {PT_OFFLINE, 0, 0};
    unsigned char b[6] = {99};
    size_t n = 91;
    CHECK(pt_dispatch(&m, PT_CONNECT, b, 2, &n) == PT_RANGE &&
          m.active == PT_OFFLINE && n == 91 && b[0] == 99);
    CHECK(pt_dispatch(&m, PT_CONNECT, b, 6, &n) == 0 && n == 3 &&
          b[0] == (0x80 | PT_OFFLINE) && b[1] == PT_ONLINE && b[2] == PT_IDLE);
    CHECK(pt_dispatch(&m, PT_START, b, 6, &n) == 0 && n == 2 &&
          b[0] == (0x80 | PT_IDLE) && b[1] == PT_BUSY);
    CHECK(pt_dispatch(&m, PT_START, b, 6, &n) == 0 && n == 2 &&
          b[0] == (0x80 | PT_BUSY) && b[1] == PT_BUSY);
    CHECK(pt_dispatch(&m, PT_PING, b, 0, &n) == 0 && n == 0 && m.pings == 1);
    CHECK(pt_dispatch(&m, PT_FAIL, b, 6, &n) == 0 && n == 3 &&
          b[0] == (0x80 | PT_BUSY) && b[1] == (0x80 | PT_ONLINE) &&
          b[2] == PT_FAULT);
    m.dispatching = 1;
    CHECK(pt_dispatch(&m, PT_RESET, b, 6, &n) == PT_STATE);
    return 0;
}

static int active(void)
{
    struct pt_active a = {{PT_OFFLINE, 0, 0}, {0}, 0, 0};
    unsigned char t[6];
    size_t n = 9;
    CHECK(pt_active_step(&a, t, 6, &n) == PT_PENDING && n == 9);
    CHECK(pt_post(&a, PT_CONNECT) == 0 && pt_post(&a, PT_START) == 0 &&
          pt_post(&a, PT_FINISH) == 0 && pt_post(&a, PT_FAIL) == 0);
    CHECK(pt_post(&a, PT_RESET) == PT_RANGE && a.count == 4 &&
          a.queue[0] == PT_CONNECT);
    CHECK(pt_active_step(&a, t, 1, &n) == PT_RANGE && a.count == 4 &&
          a.head == 0 && a.machine.active == PT_OFFLINE);
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(pt_active_step(&a, t, 6, &n) == 0);
    }
    CHECK(a.count == 0 && a.machine.active == PT_FAULT);
    return 0;
}

static int thread(void)
{
    struct pt_thread p = {0};
    uint32_t out = 91;
    CHECK(pt_thread_sum(&p, 0, 99, &out) == PT_PENDING && p.count == 0 &&
          out == 91);
    CHECK(pt_thread_sum(&p, 1, 10, &out) == PT_PENDING);
    CHECK(pt_thread_sum(&p, 0, 99, &out) == PT_PENDING && p.sum == 10);
    CHECK(pt_thread_sum(&p, 1, 20, &out) == PT_PENDING);
    CHECK(pt_thread_sum(&p, 1, 30, &out) == PT_DONE && out == 60);
    CHECK(pt_thread_sum(&p, 1, 90, &out) == PT_DONE && out == 60);
    p = (struct pt_thread){1, 1, UINT32_MAX};
    out = 91;
    CHECK(pt_thread_sum(&p, 1, 1, &out) == PT_RANGE && p.sum == UINT32_MAX &&
          p.count == 1 && out == 91);
    return 0;
}

struct pf_bt
{
    unsigned calls[8], aborts[8];
};

static unsigned tick(void *ctx, unsigned leaf)
{
    struct pf_bt *p = ctx;
    p->calls[leaf]++;
    return leaf == 1 ? PT_BT_SUCCESS : PT_BT_RUNNING;
}

static void abort_leaf(void *ctx, unsigned leaf)
{
    ((struct pf_bt *)ctx)->aborts[leaf]++;
}

static int behavior(void)
{
    struct pt_node nodes[] = {
        {PT_BT_SEQUENCE, 1, 2}, {PT_BT_LEAF, 0, 0}, {PT_BT_LEAF, 0, 0}};
    struct pt_behavior b = {0};
    struct pf_bt ctx = {0};
    unsigned result = 91;
    CHECK(pt_behavior_tick(&b, nodes, 3, tick, &ctx, &result) == 0 &&
          result == PT_BT_RUNNING && ctx.calls[1] == 1 && ctx.calls[2] == 1);
    CHECK(pt_behavior_tick(&b, nodes, 3, tick, &ctx, &result) == 0 &&
          ctx.calls[1] == 1 && ctx.calls[2] == 2);
    CHECK(pt_behavior_cancel(&b, abort_leaf, &ctx) == 0 && ctx.aborts[2] == 1 &&
          ctx.aborts[1] == 0 && b.active_mask == 0);
    CHECK(pt_behavior_cancel(&b, abort_leaf, &ctx) == 0 && ctx.aborts[2] == 1);
    nodes[0].first = 0;
    CHECK(pt_behavior_tick(&b, nodes, 3, tick, &ctx, &result) == PT_INVALID);
    return 0;
}

struct pf_device
{
    unsigned start, stop, watch, fail, quiet;
    const unsigned char *borrow;
};

static int dma(void *ctx, const unsigned char *p, size_t n)
{
    struct pf_device *d = ctx;
    (void)n;
    d->start++;
    d->borrow = p;
    return (int)d->fail;
}

static int stop(void *ctx)
{
    struct pf_device *d = ctx;
    d->stop++;
    if (d->quiet)
    {
        d->borrow = NULL;
    }
    return !d->quiet;
}

static void watchdog(void *ctx)
{
    ((struct pf_device *)ctx)->watch++;
}

static int driver(void)
{
    struct pf_device port = {0};
    struct pt_driver d = {
        {1, PT_READY, 3, 0}, {&port, dma, stop, watchdog}, NULL, 0, 0};
    unsigned char b[4] = {0};
    port.fail = 1;
    CHECK(pt_driver_start(&d, b, 4) == PT_RECOVERY &&
          d.device.state == PT_RECOVERY_REQUIRED && d.dma_owned &&
          d.buffer == b && port.borrow == b);
    CHECK(pt_driver_recover(&d) == PT_PENDING && d.dma_owned && d.buffer == b);
    port.quiet = 1;
    CHECK(pt_driver_recover(&d) == 0 && !d.dma_owned && !port.borrow &&
          d.device.state == PT_CLOSED);
    d.device.state = PT_READY;
    port.fail = 0;
    CHECK(pt_driver_start(&d, b, 4) == 0);
    CHECK(pt_driver_complete(&d, 0, 0) == PT_PENDING && d.dma_owned &&
          port.watch == 0);
    CHECK(pt_driver_complete(&d, 0, 1) == 0 && !d.dma_owned && port.watch == 1);
    CHECK(pt_driver_complete(&d, 0, 1) == PT_STATE && port.watch == 1);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
#define RUN(name)                \
    if (!strcmp(argv[1], #name)) \
    return name()
    RUN(typestate);
    RUN(builder);
    RUN(session);
    RUN(hsm);
    RUN(active);
    RUN(thread);
    RUN(behavior);
    RUN(driver);
    return 2;
}
