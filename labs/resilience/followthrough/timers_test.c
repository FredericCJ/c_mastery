#include "timers.h"
#include "cmx.h"
#include <inttypes.h>
#include <pthread.h>
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

struct rz_paused
{
    pthread_mutex_t mutex;
    struct rzt_timers timers;
    struct rzt_id ids[8];
};

static void *admit_paused(void *p)
{
    struct rz_paused *a = p;
    CHECK(!pthread_mutex_lock(&a->mutex));
    for (unsigned i = 0; i < 8; ++i)
    {
        CHECK(rzt_schedule(&a->timers, 100, i + 1, &a->ids[i]) == RZ_OK);
    }
    CHECK(!pthread_mutex_unlock(&a->mutex));
    return NULL;
}

static void structures(void)
{
    for (int kind = RZT_LIST; kind <= RZT_WHEEL; ++kind)
    {
        struct rzt_timers t;
        struct rzt_id ids[16];
        struct rzt_events events;
        CHECK(rzt_init(&t, (enum rzt_kind)kind, 0) == RZ_OK);
        CHECK(rzt_schedule(&t, 0, 9, &ids[0]) == RZ_OK);
        CHECK(rzt_cancel(&t, ids[0]) == RZ_OK);
        CHECK(rzt_schedule(&t, 0, 9, &ids[1]) == RZ_OK);
        CHECK(ids[0].slot == ids[1].slot &&
              ids[0].generation != ids[1].generation);
        CHECK(rzt_cancel(&t, ids[0]) == RZ_STALE);
        CHECK(rzt_advance(&t, 1, &events) == RZ_OK && events.count == 0);
        CHECK(rzt_advance(&t, 9, &events) == RZ_OK && events.count == 1 &&
              events.events[0].due == 9);
        CHECK(rzt_cancel(&t, ids[1]) == RZ_STALE && rzt_invariant(&t));
        CHECK(rzt_init(&t, (enum rzt_kind)kind, 0) == RZ_OK);
        for (unsigned i = 0; i < 16; ++i)
        {
            CHECK(rzt_schedule(&t, 0, 63, &ids[i]) == RZ_OK &&
                  rzt_invariant(&t));
        }
        struct rzt_id refused = {NULL, 77, 77};
        CHECK(rzt_schedule(&t, 0, 1, &refused) == RZ_FULL &&
              refused.slot == 77);
        for (unsigned i = 1; i < 16; i += 2)
        {
            CHECK(rzt_cancel(&t, ids[i]) == RZ_OK && rzt_invariant(&t));
        }
        for (unsigned i = 1; i < 16; i += 2)
        {
            CHECK(rzt_schedule(&t, 0, 63, &ids[i]) == RZ_OK &&
                  rzt_invariant(&t));
        }
        CHECK(rzt_advance(&t, 63, &events) == RZ_OK && events.count == 16 &&
              rzt_invariant(&t));
        for (unsigned i = 1; i < 16; ++i)
        {
            CHECK(events.events[i - 1].sequence < events.events[i].sequence);
        }
        CHECK(t.work.largest_expiry == 16);
        CHECK(rzt_init(&t, (enum rzt_kind)kind, UINT64_MAX - 1) == RZ_OK);
        CHECK(rzt_schedule(&t, UINT64_MAX - 1, 2, &refused) == RZ_RANGE &&
              refused.slot == 77);
        CHECK(rzt_schedule(&t, UINT64_MAX - 1, 0, &refused) == RZ_EXPIRED);
        CHECK(rzt_schedule(&t, UINT64_MAX - 1, 1, &ids[0]) == RZ_OK);
        CHECK(rzt_advance(&t, UINT64_MAX, &events) == RZ_OK &&
              events.count == 1);
        CHECK(rzt_schedule(&t, UINT64_MAX, 1, &refused) == RZ_RANGE);
        CHECK(rzt_init(&t, (enum rzt_kind)kind, 0) == RZ_OK);
        t.nodes[0].generation = UINT32_MAX - 1;
        CHECK(rzt_schedule(&t, 0, 1, &ids[0]) == RZ_OK &&
              ids[0].generation == UINT32_MAX);
        CHECK(rzt_cancel(&t, ids[0]) == RZ_OK);
        CHECK(rzt_schedule(&t, 0, 1, &ids[1]) == RZ_OK && ids[1].slot == 1);
        CHECK(rzt_cancel(&t, ids[0]) == RZ_STALE);
        t.sequence = UINT64_MAX - 1;
        CHECK(rzt_schedule(&t, 0, 1, &ids[2]) == RZ_OK);
        CHECK(rzt_schedule(&t, 0, 1, &refused) == RZ_FULL &&
              refused.slot == 77);
        events.count = 77;
        CHECK(rzt_advance(&t, 1025, &events) == RZ_RANGE &&
              events.count == 77 && t.now == 0);

        struct rz_paused paused = {PTHREAD_MUTEX_INITIALIZER, {0}, {{0}}};
        CHECK(rzt_init(&paused.timers, (enum rzt_kind)kind, 0) == RZ_OK);
        for (unsigned i = 0; i < 8; ++i)
        {
            CHECK(rzt_schedule(&paused.timers, 0, i + 1, &ids[i]) == RZ_OK);
        }
        pthread_t producer;
        CHECK(!pthread_create(&producer, NULL, admit_paused, &paused));
        CHECK(!pthread_join(producer, NULL));
        CHECK(!pthread_mutex_lock(&paused.mutex));
        CHECK(rzt_advance(&paused.timers, 100, &events) == RZ_OK &&
              events.count == 8);
        for (unsigned i = 0; i < 8; ++i)
        {
            CHECK(events.events[i].due == i + 1 &&
                  events.events[i].lateness == 99 - i);
        }
        CHECK(rzt_advance(&paused.timers, 108, &events) == RZ_OK &&
              events.count == 8);
        for (unsigned i = 0; i < 8; ++i)
        {
            CHECK(events.events[i].due == 101 + i);
        }
        CHECK(rzt_invariant(&paused.timers));
        CHECK(!pthread_mutex_unlock(&paused.mutex));
        CHECK(!pthread_mutex_destroy(&paused.mutex));
    }
    /* Real retained CMX: only the common due/cancel set, not stable event ordering. */
    for (unsigned initial = 0; initial < 2; ++initial)
    {
        cmx_kernel kernel;
        cmx_init(&kernel, CMX_RR);
        uint32_t origin = initial ? UINT32_MAX - 8 : 0;
        kernel.now = origin;
        struct rzt_timers t;
        CHECK(rzt_init(&t, RZT_WHEEL, origin) == RZ_OK);
        struct rzt_id ids[16];
        struct rzt_events events;
        for (unsigned i = 0; i < 16; ++i)
        {
            CHECK(cmx_task_add(&kernel, 1, 0, 0x1000u + i * 512u, 512) ==
                  (int)i);
            unsigned delay = (i * 7) % 16 + 1;
            CHECK(cmx_sleep(&kernel, i, cmx_at(origin + delay)) == CMX_BLOCKED);
            CHECK(rzt_schedule(&t, origin, delay, &ids[i]) == RZ_OK);
        }
        CHECK(cmx_cancel(&kernel, 7) == CMX_OK);
        CHECK(rzt_cancel(&t, ids[7]) == RZ_OK);
        unsigned fired = 0;
        for (unsigned tick = 1; tick <= 16; ++tick)
        {
            cmx_tick(&kernel, origin + tick);
            CHECK(rzt_advance(&t, (uint64_t)origin + tick, &events) == RZ_OK);
            for (unsigned i = 0; i < events.count; ++i)
            {
                fired |= 1u << events.events[i].id.slot;
            }
            for (unsigned i = 0; i < 16; ++i)
            {
                CHECK((kernel.task[i].state == CMX_READY) ==
                      ((fired & (1u << i)) != 0 || i == 7));
            }
            CHECK(cmx_invariant(&kernel));
        }
    }
}

static void oracle(void)
{
    int kind;
    char word[16];
    while (scanf("%d %15s", &kind, word) == 2)
    {
        struct rzt_timers t;
        struct rzt_id id = {NULL, 99, 99};
        uint64_t wall = 0;
        CHECK(rzt_init(&t, (enum rzt_kind)kind, 0) == RZ_OK);
        for (size_t i = 0; word[i]; ++i)
        {
            int status = RZ_OK;
            struct rzt_events events = {0};
            char op = word[i];
            if (op == 'a' || op == 'b' || op == 'c')
            {
                status = rzt_schedule(&t, wall,
                                      op == 'a'   ? 1
                                      : op == 'b' ? 9
                                                  : 63,
                                      &id);
            }
            else if (op == 'x')
            {
                status = rzt_cancel(&t, id);
            }
            else if (op == 'p')
            {
                wall += 100;
            }
            else
            {
                CHECK(op == 't' || op == 'j');
                wall += op == 't' ? 1 : 100;
                status = rzt_advance(&t, wall, &events);
            }
            CHECK(rzt_invariant(&t));
            printf("%d,%" PRIu64 ",%" PRIu64 ",%u[", status, t.now, t.observed,
                   t.count);
            for (unsigned e = 0; e < events.count; ++e)
            {
                struct rzt_event event = events.events[e];
                printf("%u:%u:%" PRIu64 ":%" PRIu64 ":%" PRIu64 ",",
                       event.id.slot, event.id.generation, event.due,
                       event.sequence, event.lateness);
            }
            printf("]; ");
        }
        puts("");
    }
}

static void pair_oracle(void)
{
    int kind;
    unsigned first, second, cancel;
    while (scanf("%d %u %u %u", &kind, &first, &second, &cancel) == 4)
    {
        struct rzt_timers t;
        struct rzt_id ids[2];
        struct rzt_events events, all = {0};
        int cancelled = RZ_INVALID;
        CHECK(rzt_init(&t, (enum rzt_kind)kind, 0) == RZ_OK);
        CHECK(rzt_schedule(&t, 0, first, &ids[0]) == RZ_OK);
        CHECK(rzt_schedule(&t, 0, second, &ids[1]) == RZ_OK);
        for (unsigned tick = 0; tick <= 16; ++tick)
        {
            if (tick == cancel)
            {
                cancelled = rzt_cancel(&t, ids[0]);
            }
            CHECK(rzt_advance(&t, tick, &events) == RZ_OK && rzt_invariant(&t));
            for (unsigned i = 0; i < events.count; ++i)
            {
                all.events[all.count++] = events.events[i];
            }
        }
        printf("%d|", cancelled);
        for (unsigned i = 0; i < all.count; ++i)
        {
            printf("%" PRIu64 ":%u,", all.events[i].due, all.events[i].id.slot);
        }
        puts("");
    }
}

static uint64_t nanos(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static void benchmark(int kind, int workload, unsigned rounds, bool latency)
{
    uint64_t checksum = 0;
    struct rzt_work sum = {0};
    uint64_t begin = nanos();
    for (unsigned round = 0; round < rounds; ++round)
    {
        struct rzt_timers t;
        struct rzt_id ids[16];
        struct rzt_events events;
        CHECK(rzt_init(&t, (enum rzt_kind)kind, 0) == RZ_OK);
        if (workload != 0)
        {
            for (unsigned i = 0; i < 16; ++i)
            {
                uint64_t delay = workload == 1   ? 63
                                 : workload == 4 ? 7
                                                 : (i * 17) % 63 + 1;
                CHECK(rzt_schedule(&t, 0, delay, &ids[i]) == RZ_OK);
            }
        }
        if (workload == 3)
        {
            for (unsigned i = 0; i < 16; i += 2)
            {
                CHECK(rzt_cancel(&t, ids[i]) == RZ_OK);
            }
        }
        for (unsigned tick = 1; tick <= 64; ++tick)
        {
            uint64_t start = latency ? nanos() : 0;
            CHECK(rzt_advance(&t, tick, &events) == RZ_OK);
            if (latency)
            {
                uint64_t duration = nanos() - start;
                printf("%d,%d,%u,%u,%u,%" PRIu64 "\n", kind, workload, round,
                       tick, events.count, duration);
            }
            for (unsigned i = 0; i < events.count; ++i)
            {
                checksum += events.events[i].due + events.events[i].sequence +
                            events.events[i].id.slot;
            }
        }
        sum.comparisons += t.work.comparisons;
        sum.link_writes += t.work.link_writes;
        sum.ticks += t.work.ticks;
        sum.cascade_visits += t.work.cascade_visits;
        if (t.work.largest_expiry > sum.largest_expiry)
        {
            sum.largest_expiry = t.work.largest_expiry;
        }
    }
    uint64_t elapsed = nanos() - begin;
    printf("{\"kind\":%d,\"workload\":%d,\"rounds\":%u,\"nanoseconds\":%" PRIu64
           ",\"checksum\":%" PRIu64 ",\"comparisons\":%" PRIu64
           ",\"link_writes\":%" PRIu64 ",\"ticks\":%" PRIu64
           ",\"cascade_visits\":%" PRIu64
           ",\"largest_expiry\":%u,\"sizeof\":%zu}\n",
           kind, workload, rounds, elapsed, checksum, sum.comparisons,
           sum.link_writes, sum.ticks, sum.cascade_visits, sum.largest_expiry,
           sizeof(struct rzt_timers));
}

int main(int argc, char **argv)
{
    CHECK(argc >= 2);
    if (!strcmp(argv[1], "test"))
    {
        structures();
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        oracle();
    }
    else if (!strcmp(argv[1], "pairs"))
    {
        pair_oracle();
    }
    else if (!strcmp(argv[1], "benchmark") || !strcmp(argv[1], "latency"))
    {
        CHECK(argc == 5);
        int kind = atoi(argv[2]), workload = atoi(argv[3]),
            rounds = atoi(argv[4]);
        CHECK(kind >= 0 && kind <= 2 && workload >= 0 && workload <= 4 &&
              rounds >= 1 && rounds <= 100000);
        benchmark(kind, workload, (unsigned)rounds,
                  !strcmp(argv[1], "latency"));
    }
    else
    {
        return 2;
    }
    return 0;
}
