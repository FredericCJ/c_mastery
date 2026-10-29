#include "composition.h"
#include "pressure.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(c)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(c))                                                            \
        {                                                                    \
            fprintf(stderr, "RZ_CHECK: %s:%d %s\n", __FILE__, __LINE__, #c); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

struct rz_memory
{
    unsigned char bytes[8192];
    size_t size;
    unsigned calls, fail_at, flushes;
    size_t written;
};

struct rz_allocator
{
    unsigned calls, made, freed, fail_at;
    void *objects[4];
    size_t sizes[4], live, peak, allocated;
};

static int size(void *p, size_t *out)
{
    *out = ((struct rz_memory *)p)->size;
    return RZ_OK;
}

static int read_bytes(void *p, size_t at, unsigned char *out, size_t n,
                      size_t *done)
{
    struct rz_memory *m = p;
    if (at > m->size)
    {
        return RZ_INVALID;
    }
    if (n > m->size - at)
    {
        n = m->size - at;
    }
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = m->bytes[at + i];
    }
    *done = n;
    return RZ_OK;
}

static int write_bytes(void *p, size_t at, const unsigned char *in, size_t n,
                       size_t *done)
{
    struct rz_memory *m = p;
    if (++m->calls == m->fail_at)
    {
        return RZJ_UNCERTAIN;
    }
    if (at > sizeof m->bytes || n > sizeof m->bytes - at)
    {
        return RZJ_UNCERTAIN;
    }
    if (n > 3)
    {
        n = 3;
    }
    for (size_t i = 0; i < n; ++i)
    {
        m->bytes[at + i] = in[i];
    }
    if (at + n > m->size)
    {
        m->size = at + n;
    }
    m->written += n;
    *done = n;
    return RZ_OK;
}

static int truncate_bytes(void *p, size_t n)
{
    struct rz_memory *m = p;
    if (n > m->size)
    {
        return RZ_INVALID;
    }
    m->size = n;
    return RZ_OK;
}

static int flush(void *p)
{
    ++((struct rz_memory *)p)->flushes;
    return RZ_OK;
}

static struct rzj_io io(struct rz_memory *m)
{
    return (struct rzj_io){m,           size,           read_bytes,
                           write_bytes, truncate_bytes, flush};
}

static void *allocate(void *p, size_t n)
{
    struct rz_allocator *a = p;
    ++a->calls;
    if (a->calls == a->fail_at)
    {
        return NULL;
    }
    void *v = malloc(n);
    if (v)
    {
        ++a->made;
        unsigned slot = 0;
        while (slot < 4 && a->objects[slot])
        {
            ++slot;
        }
        CHECK(slot < 4);
        a->objects[slot] = v;
        a->sizes[slot] = n;
        a->live += n;
        a->allocated += n;
        if (a->live > a->peak)
        {
            a->peak = a->live;
        }
    }
    return v;
}

static void deallocate(void *p, void *v)
{
    struct rz_allocator *a = p;
    unsigned slot = 0;
    while (slot < 4 && a->objects[slot] != v)
    {
        ++slot;
    }
    CHECK(slot < 4);
    a->live -= a->sizes[slot];
    a->objects[slot] = NULL;
    ++a->freed;
    free(v);
}

static int run(struct rzc_config config, unsigned allocation_failure,
               unsigned write_failure, struct rzc_result *out)
{
    struct rz_memory journal = {.fail_at = write_failure}, ack = {0};
    struct rz_allocator allocator = {.fail_at = allocation_failure};
    int status =
        rzc_run(config, io(&journal), io(&ack),
                (struct rzf_allocator){&allocator, allocate, deallocate}, out);
    CHECK(allocator.made == allocator.freed);
    return status;
}

static void tests(void)
{
    struct rzc_config config = {{0, 0}, {1, 1}, 10, 0, 0, 4, 0, false, false};
    struct rzc_result result = {0};
    CHECK(run(config, 0, 0, &result) == RZ_OK && result.total == 3 &&
          result.applied == 1 && result.started == 1 &&
          result.dispatched == 1 && result.winner == 0 &&
          result.reply_at == 1 && result.quiet_at == 1 &&
          result.deliveries == 1 && result.acknowledged == 1 &&
          result.tokens == 3);
    config.drops[0] = 1;
    config.retry_after = 1501;
    CHECK(run(config, 0, 0, &result) == RZ_OK && result.applied == 1 &&
          result.reply_at == 3 && result.dispatched == 2);
    config.drops[0] = 0;
    config.retry_after = 0;
    config.hedge = true;
    config.delay[0] = 4;
    config.delay[1] = 1;
    config.reset_before_ack = true;
    CHECK(run(config, 0, 0, &result) == RZ_OK && result.applied == 1 &&
          result.started == 2 && result.winner == 1 && result.reply_at == 3 &&
          result.quiet_at == 4 && result.deliveries == 2 && result.resets == 1);
    for (unsigned mask = 0; mask < 16; ++mask)
    {
        for (unsigned second = 0; second < 16; ++second)
        {
            for (unsigned deadline = 1; deadline <= 16; ++deadline)
            {
                config = (struct rzc_config){{mask, second},
                                             {1 + mask % 8, 1 + second % 8},
                                             deadline,
                                             mask % 8,
                                             second % 5,
                                             1 + mask % 8,
                                             mask % 2 ? 1501 : 0,
                                             true,
                                             (second & 1) != 0};
                CHECK(run(config, 0, 0, &result) == RZ_OK);
                CHECK(result.applied <= 1 &&
                      result.total == (result.applied ? 3 : 0) &&
                      result.dispatched <= config.capacity &&
                      result.tokens + result.dispatched == config.capacity &&
                      result.allocated_responses == result.freed_responses);
                CHECK(result.deliveries ==
                          result.applied * (1u + config.reset_before_ack) &&
                      result.acknowledged == result.applied &&
                      result.quiet_at >= result.reply_at);
                if (result.winner >= 0)
                {
                    CHECK(result.reply_at < deadline &&
                          (!config.cancel_at ||
                           result.reply_at < config.cancel_at));
                }
            }
        }
    }
    config = (struct rzc_config){{0, 0}, {4, 1}, 10, 0, 0, 4, 0, true, true};
    for (unsigned failure = 1; failure <= 4; ++failure)
    {
        result = (struct rzc_result){.total = 99};
        int status = run(config, failure, 0, &result);
        CHECK(status == (failure == 1 ? RZ_FULL : RZ_OK));
        if (failure == 1)
        {
            CHECK(result.total == 99);
        }
        else
        {
            CHECK(result.applied == 1 &&
                  result.allocated_responses == result.freed_responses);
        }
    }
    for (unsigned failure = 1; failure <= 45; ++failure)
    {
        result = (struct rzc_result){.total = 99};
        int status = run(config, 0, failure, &result);
        CHECK(status == RZ_OK || status == RZJ_UNCERTAIN);
        if (status != RZ_OK)
        {
            CHECK(result.total == 99);
        }
    }
    puts("composition passed");
}

static void oracle(void)
{
    struct rzc_config c;
    unsigned hedge, reset;
    while (scanf("%u %u %u %u %u %u %u %u %u %u %u", &c.drops[0], &c.drops[1],
                 &c.delay[0], &c.delay[1], &c.deadline, &c.notify_at,
                 &c.cancel_at, &c.capacity, &c.retry_after, &hedge,
                 &reset) == 11)
    {
        c.hedge = hedge != 0;
        c.reset_before_ack = reset != 0;
        struct rzc_result r = {0};
        int status = run(c, 0, 0, &r);
        printf("%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%" PRIu64
               ",%d,%d,%u,%u\n",
               status, r.started, r.dispatched, r.retries, r.dropped, r.denied,
               r.applied, r.replays, r.deliveries, r.acknowledged, r.resets,
               r.reply_at, r.quiet_at, r.notification_at, r.tokens, r.winner,
               r.total, r.journal_records, r.ack_records);
    }
}

static struct rzc_config workload(unsigned mode)
{
    struct rzc_config c = {{0, 0}, {1, 1}, 16, 0, 0, 8, 0, false, false};
    if (mode == 1)
    {
        c.drops[0] = 3;
    }
    if (mode == 2 || mode == 3)
    {
        c.delay[0] = 8;
        c.delay[1] = 1;
        c.hedge = mode == 3;
    }
    if (mode == 4 || mode == 5)
    {
        c.drops[0] = 15;
        c.drops[1] = 15;
        c.hedge = mode == 5;
    }
    if (mode == 6)
    {
        c.notify_at = 12;
        c.reset_before_ack = true;
    }
    if (mode == 7)
    {
        c.delay[0] = 8;
        c.delay[1] = 4;
        c.hedge = true;
        c.cancel_at = 3;
    }
    if (mode == 8)
    {
        c.capacity = 1;
        c.delay[0] = 8;
        c.hedge = true;
    }
    if (mode == 9)
    {
        c.drops[0] = 1;
        c.retry_after = 1501;
    }
    return c;
}

static uint64_t nanoseconds(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}

static void benchmark(unsigned mode, unsigned rounds)
{
    CHECK(mode < 10 && rounds > 0 && rounds <= 10000);
    struct rzc_config c = workload(mode);
    struct rzc_result r = {0};
    struct rz_memory journal = {0}, ack = {0};
    struct rz_allocator a = {0};
    uint64_t checksum = 0, start = nanoseconds();
    for (unsigned i = 0; i < rounds; ++i)
    {
        journal = (struct rz_memory){0};
        ack = (struct rz_memory){0};
        a = (struct rz_allocator){0};
        CHECK(rzc_run(c, io(&journal), io(&ack),
                      (struct rzf_allocator){&a, allocate, deallocate},
                      &r) == RZ_OK);
        CHECK(a.made == a.freed && !a.live && r.applied <= 1 &&
              r.total == (r.applied ? 3 : 0));
        checksum += r.dispatched + r.reply_at + r.quiet_at + r.deliveries +
                    r.acknowledged + (unsigned)r.total;
    }
    uint64_t elapsed = nanoseconds() - start;
    printf(
        "{\"mode\":%u,\"rounds\":%u,\"nanoseconds\":%" PRIu64
        ",\"checksum\":%" PRIu64
        ",\"dispatched\":%u,\"retries\":%u,\"dropped\":%u,\"denied\":%u,\"winner\":%d,\"reply_tick\":%u,\"quiet_tick\":%u,\"notification_tick\":%u,\"deliveries\":%u,\"acknowledged\":%u,\"applied\":%u,\"replays\":%u,\"tokens\":%" PRIu64
        ",\"allocations\":%u,\"allocated_bytes\":%zu,\"peak_heap_bytes\":%zu,\"journal_write_calls\":%u,\"journal_bytes_written\":%zu,\"journal_flushes\":%u,\"ack_write_calls\":%u,\"ack_bytes_written\":%zu,\"ack_flushes\":%u,\"timer_comparisons\":%" PRIu64
        ",\"timer_links\":%" PRIu64 ",\"timer_ticks\":%" PRIu64
        ",\"timer_cascade\":%" PRIu64 "}\n",
        mode, rounds, elapsed, checksum, r.dispatched, r.retries, r.dropped,
        r.denied, r.winner, r.reply_at, r.quiet_at, r.notification_at,
        r.deliveries, r.acknowledged, r.applied, r.replays, r.tokens, a.made,
        a.allocated, a.peak, journal.calls, journal.written, journal.flushes,
        ack.calls, ack.written, ack.flushes, r.timer_work.comparisons,
        r.timer_work.link_writes, r.timer_work.ticks,
        r.timer_work.cascade_visits);
}

static void layout(void)
{
    printf(
        "{\"scope\":%zu,\"scope_align\":%zu,\"bucket\":%zu,\"timers\":%zu,\"retry\":%zu,\"tcp\":%zu,\"arbiter\":%zu,\"journal\":%zu,\"outbox\":%zu,\"hub\":%zu,\"service_io\":%zu,\"result\":%zu,\"fixture_device\":%zu,\"owned_queue\":%zu,\"pressure_pump\":%zu,\"gate\":%zu}\n",
        sizeof(struct rzf_scope), _Alignof(struct rzf_scope),
        sizeof(struct rzf_bucket), sizeof(struct rzt_timers),
        sizeof(struct rzr_retry), sizeof(struct net_stack),
        sizeof(struct rzh_arbiter), sizeof(struct rzj_journal),
        sizeof(struct rzs_outbox), sizeof(struct msg_hub),
        sizeof(struct rzv_service), sizeof(struct rzc_result),
        sizeof(struct rz_memory), sizeof(struct rzp_owned_queue),
        sizeof(struct rzp_pump), sizeof(struct rzb_gate));
}

int main(int argc, char **argv)
{
    if (argc == 4 && !strcmp(argv[1], "benchmark"))
    {
        benchmark((unsigned)strtoul(argv[2], NULL, 10),
                  (unsigned)strtoul(argv[3], NULL, 10));
        return 0;
    }
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "test"))
    {
        tests();
        return 0;
    }
    if (!strcmp(argv[1], "oracle"))
    {
        oracle();
        return 0;
    }
    if (!strcmp(argv[1], "layout"))
    {
        layout();
        return 0;
    }
    return 2;
}
