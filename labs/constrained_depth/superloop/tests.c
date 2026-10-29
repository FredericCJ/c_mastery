#include "superloop.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("SC_CHECK:%u:%s\n", (unsigned)__LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)

static int bounds(void)
{
    uint64_t original[3] = {2, 7, 1}, sliced[3] = {2, 2, 1};
    struct sc_bound b = {99, 99, 99};
    CHECK(sc_bound(original, 3, 1, 1, &b) == SC_OK && b.round == 10 &&
          b.longest == 7 && b.response == 11);
    CHECK(sc_bound(sliced, 3, 1, 1, &b) == SC_OK && b.round == 5 &&
          b.longest == 2 && b.response == 6);
    struct sc_bound before = b;
    CHECK(sc_bound(sliced, 3, 1, 0, &b) == SC_UNBOUNDED &&
          !memcmp(&b, &before, sizeof b));
    uint64_t large[2] = {UINT64_MAX, 1};
    CHECK(sc_bound(large, 2, 0, 1, &b) == SC_RANGE &&
          !memcmp(&b, &before, sizeof b));
    CHECK(sc_bound(large, 1, 1, 1, &b) == SC_RANGE &&
          !memcmp(&b, &before, sizeof b));
    return 0;
}

static int continuations(void)
{
    unsigned char source[100], result[128];
    for (unsigned i = 0; i < 100; i++)
    {
        source[i] = (unsigned char)(0x61 + i % 26);
    }
    struct sc_task tasks[3];
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(sc_task_init(tasks + i, source, 100) == SC_OK);
    }
    memset(source, 0, sizeof source);
    size_t ticks[3] = {0};
    for (unsigned round = 0; round < 50; round++)
    {
        for (unsigned i = 0; i < 3; i++)
        {
            size_t work = 99;
            CHECK(sc_task_step(tasks + i, 2, &work) == SC_OK && work == 2);
            ticks[i] += work;
            CHECK(tasks[i].cursor == (round + 1) * 2);
        }
    }
    for (unsigned i = 0; i < 3; i++)
    {
        size_t count = 99, work = 99;
        CHECK(ticks[i] == 100 && tasks[i].emissions == 1);
        CHECK(sc_task_step(tasks + i, 1, &work) == SC_CLOSED && work == 99 &&
              tasks[i].emissions == 1);
        CHECK(sc_task_result(tasks + i, result, 100, &count) == SC_OK &&
              count == 100);
        for (unsigned k = 0; k < 100; k++)
        {
            CHECK(result[k] == 0x41 + k % 26);
        }
    }
    for (unsigned cut = 0; cut <= 50; cut++)
    {
        for (unsigned i = 0; i < 100; i++)
        {
            source[i] = (unsigned char)(0x61 + i % 26);
        }
        struct sc_task task;
        CHECK(sc_task_init(&task, source, 100) == SC_OK);
        size_t work = 0, count = 99;
        for (unsigned i = 0; i < cut; i++)
        {
            CHECK(sc_task_step(&task, 2, &work) == SC_OK);
        }
        memset(result, 91, sizeof result);
        if (cut < 50)
        {
            CHECK(sc_task_cancel(&task) == SC_OK &&
                  task.state == SC_CANCELLED && task.emissions == 0);
            CHECK(sc_task_result(&task, result, sizeof result, &count) ==
                      SC_CLOSED &&
                  count == 99 && result[0] == 91);
            CHECK(sc_task_step(&task, 2, &work) == SC_CLOSED);
        }
        else
        {
            CHECK(sc_task_cancel(&task) == SC_CLOSED && task.emissions == 1);
        }
    }
    struct sc_task empty;
    CHECK(sc_task_init(&empty, NULL, 0) == SC_OK && empty.emissions == 1);
    size_t count = 99;
    CHECK(sc_task_result(&empty, result, 0, &count) == SC_OK && count == 0);
    puts("SC_CONTINUATIONS:three-tasks=3 cancel-yields=51 exact-once=1");
    return 0;
}

static int events(void)
{
    struct sc_events e;
    struct sc_latest latest;
    CHECK(sc_events_init(&e) == SC_OK && sc_latest_init(&latest) == SC_OK);
    for (uint64_t i = 1; i <= 5; i++)
    {
        CHECK(sc_events_push(&e, (struct sc_event){i, i * 10}) ==
              (i <= 3 ? SC_OK : SC_FULL));
        CHECK(sc_latest_put(&latest, i * 10) == SC_OK);
    }
    CHECK(e.accepted == 3 && e.rejected == 2 && e.count == 3 &&
          latest.updates == 5 && latest.superseded == 4);
    struct sc_event out = {99, 99};
    for (uint64_t i = 1; i <= 3; i++)
    {
        CHECK(sc_events_pop(&e, &out) == SC_OK && out.id == i &&
              out.value == i * 10);
    }
    CHECK(sc_events_pop(&e, &out) == SC_EMPTY && out.id == 3);
    uint64_t value = 99;
    CHECK(sc_latest_take(&latest, &value) == SC_OK && value == 50);
    CHECK(sc_latest_take(&latest, &value) == SC_EMPTY && value == 50);
    e.accepted = UINT64_MAX;
    CHECK(sc_events_push(&e, (struct sc_event){6, 60}) == SC_RANGE &&
          e.count == 0);
    latest.updates = UINT64_MAX;
    CHECK(sc_latest_put(&latest, 60) == SC_RANGE && latest.pending == 0);
    return 0;
}

static int periodic(void)
{
    for (unsigned policy = 0; policy < 2; policy++)
    {
        struct sc_releases r;
        CHECK(sc_releases_init(&r, 4, 0, policy) == SC_OK);
        CHECK(sc_releases_poll(&r, 0) == SC_OK &&
              sc_releases_poll(&r, 4) == SC_OK &&
              sc_releases_poll(&r, 8) == SC_OK &&
              sc_releases_poll(&r, 20) == SC_OK);
        CHECK(r.seen == 6 && r.lost == 5 && r.count == 1 && r.items[0] == 20 &&
              r.next == 24);
        CHECK(sc_releases_poll(&r, 21) == SC_OK &&
              r.count == (policy == SC_COALESCE ? 1u : 0u) &&
              r.lost == (policy == SC_COALESCE ? 5u : 6u));
        struct sc_releases saved = r;
        CHECK(sc_releases_poll(&r, 19) == SC_INVALID &&
              !memcmp(&r, &saved, sizeof r));
    }
    struct sc_releases r;
    CHECK(sc_releases_init(&r, 4, 0, SC_LOSSLESS) == SC_OK &&
          sc_releases_poll(&r, 4) == SC_OK && r.count == 2);
    struct sc_releases before = r;
    CHECK(sc_releases_poll(&r, 20) == SC_FULL &&
          !memcmp(&r, &before, sizeof r));
    uint64_t value = 99;
    CHECK(sc_releases_pop(&r, &value) == SC_OK && value == 0);
    CHECK(sc_releases_pop(&r, &value) == SC_OK && value == 4);
    CHECK(sc_releases_poll(&r, 8) == SC_OK &&
          sc_releases_pop(&r, &value) == SC_OK && value == 8);
    before = r;
    CHECK(sc_releases_poll(&r, 20) == SC_FULL &&
          !memcmp(&r, &before, sizeof r));
    CHECK(sc_releases_poll(&r, 16) == SC_OK && r.count == 2 &&
          r.items[0] == 12 && r.items[1] == 16);
    CHECK(sc_releases_init(&r, 1, UINT64_MAX, SC_LOSSLESS) == SC_OK &&
          sc_releases_poll(&r, UINT64_MAX) == SC_OK && r.exhausted &&
          r.count == 1);
    CHECK(sc_releases_pop(&r, &value) == SC_OK && value == UINT64_MAX &&
          sc_releases_poll(&r, UINT64_MAX) == SC_OK && r.count == 0);
    CHECK(sc_releases_init(&r, 1, 0, SC_COALESCE) == SC_OK);
    before = r;
    CHECK(sc_releases_poll(&r, UINT64_MAX) == SC_RANGE &&
          !memcmp(&r, &before, sizeof r));
    puts(
        "SC_RELEASES:period=4 seen=6 coalesced=5 reject-stale=6 lossless-refusal=1");
    return 0;
}

static int fairness(void)
{
    for (unsigned mode = 0; mode < 3; mode++)
    {
        struct sc_schedule s;
        CHECK(sc_schedule_init(&s, mode) == SC_OK);
        unsigned counts[4] = {0}, last[3] = {0}, maxgap = 0;
        for (unsigned t = 0; t < 40; t++)
        {
            unsigned selected = 99;
            CHECK(sc_schedule_next(&s, t, mode == 0 ? 7 : 15, &selected) ==
                  SC_OK);
            counts[selected]++;
            if (mode == 0)
            {
                CHECK(selected == t % 3);
            }
            if (mode == 1)
            {
                CHECK(selected == 3);
            }
            if (mode == 2)
            {
                CHECK(selected == (t % 4 == 0 ? 3 : t % 4 - 1));
                if (selected < 3)
                {
                    if (counts[selected] > 1 && t - last[selected] > maxgap)
                    {
                        maxgap = t - last[selected];
                    }
                    last[selected] = t;
                }
            }
        }
        if (mode == 2)
        {
            CHECK(counts[0] == 10 && counts[1] == 10 && counts[2] == 10 &&
                  counts[3] == 10 && maxgap == 4);
        }
        struct sc_schedule before = s;
        unsigned out = 99;
        CHECK(sc_schedule_next(&s, 39, 15, &out) == SC_INVALID && out == 99 &&
              !memcmp(&s, &before, sizeof s));
    }
    puts("SC_FAIRNESS:equal=ABC priority=HHH budgeted=HABC low-max-gap=4");
    return 0;
}

static int latency(void)
{
    unsigned char input[100];
    memset(input, 0x61, sizeof input);
    for (unsigned quota = 2; quota <= 100; quota += 98)
    {
        struct sc_task task;
        CHECK(sc_task_init(&task, input, sizeof input) == SC_OK);
        unsigned now = 0, largest = 0, rounds = 0;
        while (task.state == SC_READY)
        {
            size_t work = 99;
            unsigned polled = now;
            now += 2;
            CHECK(sc_task_step(&task, quota, &work) == SC_OK);
            now += (unsigned)work;
            now++;
            unsigned gap = now - polled;
            if (gap > largest)
            {
                largest = gap;
            }
            rounds++;
        }
        CHECK(largest == (quota == 2 ? 5u : 103u) &&
              rounds == (quota == 2 ? 50u : 1u));
        printf(
            "SC_LATENCY:quota=%u rounds=%u max-poll-gap=%u completed-work=100\n",
            quota, rounds, largest);
    }
    return 0;
}

int main(void)
{
    if (bounds() || continuations() || events() || periodic() || fairness() ||
        latency())
    {
        return 1;
    }
    puts("SC_PASS:bounded-superloop-owned-continuations-and-overload");
    return 0;
}
