#include "collection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "COLLECTION_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                                   \
        }                                                              \
    } while (0)
#define N 17

struct rd_fixture
{
    struct tc_collection q;
    struct td_timer timers[N];
    size_t index[N], pos[N], next[N], prev[N], heads[7];
};

static void init(struct rd_fixture *f, enum tc_kind kind, unsigned limit)
{
    CHECK(tc_init(&f->q, kind, f->timers, N, f->index, f->pos, f->next, f->prev,
                  f->heads, 7, 3, limit) == 0);
}

static void membership(struct rd_fixture *f)
{
    size_t live = 0;
    unsigned seen[N] = {0};
    for (size_t i = 0; i < N; ++i)
    {
        live += f->timers[i].state == TD_ARMED;
    }
    CHECK(live == f->q.count);
    if (f->q.kind == TC_HEAP)
    {
        for (size_t i = 0; i < live; ++i)
        {
            size_t slot = f->index[i];
            CHECK(slot < N && !seen[slot]++ && f->pos[slot] == i &&
                  f->timers[slot].state == TD_ARMED);
            if (i)
            {
                size_t parent = f->index[(i - 1) / 2];
                CHECK(f->timers[parent].deadline < f->timers[slot].deadline ||
                      (f->timers[parent].deadline == f->timers[slot].deadline &&
                       parent < slot));
            }
        }
    }
    else if (f->q.kind == TC_WHEEL)
    {
        size_t total = 0;
        for (size_t b = 0; b < 7; ++b)
        {
            size_t prior = SIZE_MAX;
            for (size_t s = f->heads[b]; s != SIZE_MAX; s = f->next[s])
            {
                CHECK(s < N && ++total <= N && !seen[s]++ && f->pos[s] == b &&
                      f->prev[s] == prior);
                CHECK(f->timers[s].state == TD_ARMED &&
                      (f->timers[s].deadline / 3) % 7 == b);
                if (prior != SIZE_MAX)
                {
                    CHECK(f->timers[prior].deadline < f->timers[s].deadline ||
                          (f->timers[prior].deadline == f->timers[s].deadline &&
                           prior < s));
                }
                prior = s;
            }
        }
        CHECK(total == live);
    }
}

/* Independent one-shot oracle: a deadline array and monotonically increasing
 * identity array, with no heap, wheel, TdTimer calls or copied expected trace. */
static void histories(enum tc_kind kind)
{
    struct rd_fixture f;
    init(&f, kind, 65535);
    uint64_t deadlines[N] = {0};
    unsigned generations[N] = {0};
    uint32_t random = 97;
    for (unsigned step = 0; step < 12000; ++step)
    {
        random = random * 1664525u + 1013904223u;
        size_t slot = (random >> 8) % N;
        unsigned op = random % 5;
        struct tc_token token = {slot, generations[slot]}, out = {99, 99};
        if (op < 3)
        {
            uint64_t d = 1 + (random >> 16) % 97;
            CHECK(tc_arm(&f.q, slot, d, 0, 0, &out) == 1);
            deadlines[slot] = d;
            ++generations[slot];
            CHECK(out.slot == slot && out.generation == generations[slot]);
            if (token.generation)
            {
                CHECK(tc_cancel(&f.q, token) == 0);
            }
        }
        else if (op == 3)
        {
            CHECK(tc_cancel(&f.q, token) == (deadlines[slot] ? 1 : 0));
            deadlines[slot] = 0;
        }
        else
        {
            size_t best = N;
            for (size_t i = 0; i < N; ++i)
            {
                if (deadlines[i] &&
                    (best == N || deadlines[i] < deadlines[best]))
                {
                    best = i;
                }
            }
            struct td_dispatch dispatch = {0};
            uint64_t now = (random >> 16) % 100;
            int expected = best != N && deadlines[best] <= now;
            CHECK(tc_begin(&f.q, now, &out, &dispatch) == expected);
            if (expected)
            {
                CHECK(out.slot == best && out.generation == generations[best] &&
                      dispatch.scheduled == deadlines[best]);
                CHECK(dispatch.effects == 1 && dispatch.expirations == 1 &&
                      dispatch.next == 0);
                CHECK(tc_finish(&f.q, out) == 1 && tc_finish(&f.q, out) == 0);
                deadlines[best] = 0;
            }
            else
            {
                CHECK(out.slot == 99 && out.generation == 99);
            }
        }
        membership(&f);
        size_t best = N;
        for (size_t i = 0; i < N; ++i)
        {
            if (deadlines[i] && (best == N || deadlines[i] < deadlines[best]))
            {
                best = i;
            }
        }
        uint64_t d = 999;
        out = (struct tc_token){99, 99};
        CHECK(tc_peek(&f.q, &out, &d) == (best != N));
        if (best != N)
        {
            CHECK(out.slot == best && d == deadlines[best]);
        }
        else
        {
            CHECK(out.slot == 99 && d == 999);
        }
    }
}

static void boundaries(enum tc_kind kind)
{
    struct rd_fixture f;
    init(&f, kind, 3);
    struct tc_token a = {99, 99}, b, current;
    struct td_dispatch d;
    CHECK(tc_arm(&f.q, N, 10, 0, 0, &a) == -1 && a.slot == 99);
    CHECK(tc_arm(&f.q, 0, 0, 0, 0, &a) == -1 && a.slot == 99);
    CHECK(tc_arm(&f.q, 0, 10, 3, 0, &a) == 1);
    CHECK(tc_arm(&f.q, 1, 10, 0, 0, &b) == 1);
    CHECK(tc_begin(&f.q, 20, &current, &d) == 1 && current.slot == 0 &&
          d.effects == 4 && d.next == 22);
    CHECK(tc_cancel(&f.q, a) == 2);
    struct tc_token unchanged = {99, 99};
    CHECK(tc_arm(&f.q, 0, 15, 0, 0, &unchanged) == 0 && unchanged.slot == 99);
    CHECK(tc_finish(&f.q, a) == 1 && f.timers[0].state == TD_IDLE);
    CHECK(tc_begin(&f.q, 20, &current, &d) == 1 && current.slot == 1 &&
          tc_finish(&f.q, current) == 1);
    CHECK(tc_arm(&f.q, 0, UINT64_MAX, 1, 1, &a) == 1);
    CHECK(tc_begin(&f.q, UINT64_MAX, &current, &d) == 1 && d.exhausted &&
          d.next == 0);
    CHECK(tc_finish(&f.q, current) == 1);
    CHECK(tc_arm(&f.q, 0, 2, 3, 1, &a) == 1);
    CHECK(tc_arm(&f.q, 0, 8, 0, 0, &unchanged) == 0 && unchanged.slot == 99);
    CHECK(tc_begin(&f.q, 9, &current, &d) == 1 && d.effects == 1 &&
          d.expirations == 3 && d.next == 11);
    CHECK(tc_finish(&f.q, current) == 1);
    uint64_t deadline;
    CHECK(tc_peek(&f.q, &current, &deadline) == 1 && deadline == 11);
    CHECK(tc_cancel(&f.q, current) == 1 &&
          tc_arm(&f.q, 0, 1, 0, 0, &unchanged) == 0);
    CHECK(f.timers[0].state == TD_RETIRED);
    membership(&f);
    /* Every equal key must dispatch by immutable slot order, across wheel turns. */
    init(&f, kind, 3);
    for (size_t i = N; i > 0; --i)
    {
        CHECK(tc_arm(&f.q, i - 1, UINT64_MAX - 1, 0, 0, &a) == 1);
    }
    for (size_t i = 0; i < N; ++i)
    {
        CHECK(tc_begin(&f.q, UINT64_MAX, &current, &d) == 1 &&
              current.slot == i);
        CHECK(tc_finish(&f.q, current) == 1);
        membership(&f);
    }
}

int main(void)
{
    for (enum tc_kind kind = TC_SCAN; kind <= TC_WHEEL; ++kind)
    {
        histories(kind);
        boundaries(kind);
    }
    puts(
        "COLLECTION_PASS:independent_histories=36000;representations=3;identity;borrow;overflow;ties;membership");
    return 0;
}
