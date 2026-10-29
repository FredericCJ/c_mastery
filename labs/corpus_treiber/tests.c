#include "treiber.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "TS_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

static void stack(void)
{
    struct ts_stack s;
    uint32_t out = 77;
    CHECK(ts_init(&s, 0) == TS_INVALID);
    CHECK(ts_init(&s, 8) == TS_OK);
    CHECK(ts_pop(&s, 0, &out) == TS_EMPTY && out == 77);
    for (unsigned i = 0; i < 8; i++)
    {
        CHECK(ts_push(&s, i + 20) == TS_OK);
    }
    CHECK(ts_push(&s, 99) == TS_FULL);
    for (unsigned i = 8; i; i--)
    {
        CHECK(ts_pop(&s, 0, &out) == TS_OK && out == i + 19);
    }
    CHECK(ts_push(&s, 99) == TS_FULL);
    CHECK(ts_collect(&s) == 8);
    CHECK(ts_push(&s, 99) == TS_OK);
    struct ts_pop a, b;
    CHECK(ts_begin(&s, 0, &a) == TS_OK && ts_begin(&s, 1, &b) == TS_OK);
    CHECK(ts_commit(&a, &out) == TS_OK && out == 99);
    out = 123;
    CHECK(ts_commit(&b, &out) == TS_RETRY && out == 123);
    CHECK(!b.owner);
    CHECK(ts_cancel(&b) == TS_INVALID);
    CHECK(ts_collect(&s) == 1);
    CHECK(ts_push(&s, UINT32_MAX) == TS_OK);
    CHECK(ts_begin(&s, 0, &a) == TS_OK);
    CHECK(ts_commit(&a, 0) == TS_INVALID && a.owner == &s);
    CHECK(ts_cancel(&a) == TS_OK);
    CHECK(ts_pop(&s, 0, &out) == TS_OK && out == UINT32_MAX);
    CHECK(ts_pop(&s, TS_THREADS, &out) == TS_INVALID && out == UINT32_MAX);
}

static void reclaim(void)
{
    struct ts_stack s;
    struct ts_pop a;
    struct ts_retired old, other, newer;
    uint32_t out;
    CHECK(ts_init(&s, 1) == TS_OK);
    CHECK(ts_push(&s, 41) == TS_OK);
    CHECK(ts_begin(&s, 0, &a) == TS_OK);
    CHECK(ts_pop(&s, 1, &out) == TS_OK && out == 41);
    CHECK(ts_collect(&s) == 0);
    CHECK(ts_push(&s, 42) == TS_FULL); /* stalled hazard */
    CHECK(ts_commit(&a, &out) == TS_RETRY);
    CHECK(ts_scan(&s, 0, &old) == TS_OK);
    CHECK(ts_scan(&s, 0, &other) == TS_OK);
    CHECK(ts_reclaim(&other) == TS_OK);
    CHECK(ts_push(&s, 42) == TS_OK);
    CHECK(ts_begin(&s, 0, &a) == TS_OK);
    CHECK(ts_pop(&s, 1, &out) == TS_OK && out == 42);
    CHECK(ts_reclaim(&old) ==
          TS_RETRY); /* must not free a later protected incarnation */
    CHECK(ts_scan(&s, 0, &newer) == TS_RETRY);
    CHECK(ts_cancel(&a) == TS_OK);
    CHECK(ts_collect(&s) == 1);
    if (TS_GEN_MAX < 16u)
    {
        for (unsigned i = 2; i <= TS_GEN_MAX; i++)
        {
            CHECK(ts_push(&s, i) == TS_OK);
            CHECK(ts_pop(&s, 0, &out) == TS_OK && out == i);
            CHECK(ts_collect(&s) == (i < TS_GEN_MAX ? 1u : 0u));
        }
        CHECK(ts_push(&s, 99) == TS_FULL);
        CHECK(ts_pop(&s, 0, &out) == TS_EMPTY);
    }
    /* Original all-atomic sequential ABA specimen: this is logical corruption,
       not a use-after-free execution or arbitrary-code-execution claim. */
    atomic_uint head, next[3];
    atomic_init(&head, 0);
    atomic_init(&next[0], 1);
    atomic_init(&next[1], 2);
    atomic_init(&next[2], TS_NONE);
    unsigned saved = atomic_load(&head), saved_next = atomic_load(&next[saved]);
    atomic_store(&head, 1);
    atomic_store(&head, 2);
    atomic_store(&next[0], 2);
    atomic_store(&head, 0);
    CHECK(atomic_compare_exchange_strong(&head, &saved, saved_next));
    CHECK(atomic_load(&head) == 1);
}

enum
{
    ITEMS = 1000
};
static struct ts_stack shared;
static atomic_uint seen[TS_THREADS * ITEMS];

static void *worker(void *arg)
{
    unsigned p = *(unsigned *)arg;
    for (unsigned i = 0; i < ITEMS; i++)
    {
        unsigned tries = 0;
        enum ts_status r;
        do
        {
            r = ts_push(&shared, p * ITEMS + i);
            if (r == TS_FULL)
            {
                (void)ts_collect(&shared);
            }
            CHECK(++tries < 10000000u);
        } while (r == TS_FULL);
        CHECK(r == TS_OK);
        uint32_t v;
        tries = 0;
        do
        {
            r = ts_pop(&shared, p, &v);
            CHECK(++tries < 10000000u);
        } while (r == TS_EMPTY);
        CHECK(r == TS_OK && v < TS_THREADS * ITEMS);
        CHECK(atomic_fetch_add(&seen[v], 1) == 0);
        (void)ts_collect(&shared);
    }
    return 0;
}

static void native(void)
{
    pthread_t threads[TS_THREADS];
    unsigned ids[TS_THREADS];
    CHECK(ts_init(&shared, 8) == TS_OK);
    for (unsigned i = 0; i < TS_THREADS * ITEMS; i++)
    {
        atomic_init(&seen[i], 0);
    }
    for (unsigned i = 0; i < TS_THREADS; i++)
    {
        ids[i] = i;
        CHECK(!pthread_create(&threads[i], 0, worker, &ids[i]));
    }
    for (unsigned i = 0; i < TS_THREADS; i++)
    {
        CHECK(!pthread_join(threads[i], 0));
    }
    for (unsigned i = 0; i < TS_THREADS * ITEMS; i++)
    {
        CHECK(atomic_load(&seen[i]) == 1);
    }
    uint32_t v;
    CHECK(ts_pop(&shared, 0, &v) == TS_EMPTY);
    (void)ts_collect(&shared);
}

struct ts_record
{
    unsigned start, end;
    enum ts_status result;
    uint32_t value;
};
static struct ts_record records[4];
static atomic_uint clock_tick;

static void *history_worker(void *arg)
{
    unsigned p = *(unsigned *)arg;
    struct ts_record *a = &records[p * 2], *b = a + 1;
    a->start = atomic_fetch_add(&clock_tick, 1);
    a->value = 10 + p;
    a->result = ts_push(&shared, a->value);
    a->end = atomic_fetch_add(&clock_tick, 1);
    b->start = atomic_fetch_add(&clock_tick, 1);
    b->value = 99;
    b->result = ts_pop(&shared, p, &b->value);
    b->end = atomic_fetch_add(&clock_tick, 1);
    return 0;
}

static void history(void)
{
    for (unsigned round = 0; round < 256; round++)
    {
        pthread_t threads[2];
        unsigned ids[2] = {0, 1};
        CHECK(ts_init(&shared, 4) == TS_OK);
        atomic_init(&clock_tick, 0);
        for (unsigned p = 0; p < 2; p++)
        {
            CHECK(!pthread_create(&threads[p], 0, history_worker, &ids[p]));
        }
        for (unsigned p = 0; p < 2; p++)
        {
            CHECK(!pthread_join(threads[p], 0));
        }
        for (unsigned i = 0; i < 4; i++)
        {
            printf("%u %u %u %u%c", records[i].start, records[i].end,
                   (unsigned)records[i].result, records[i].value,
                   i == 3 ? '\n' : ' ');
        }
    }
}

static void driver(void)
{
    struct ts_stack s;
    unsigned cap, n;
    char op;
    CHECK(scanf("%u %u", &cap, &n) == 2);
    CHECK(ts_init(&s, cap) == TS_OK);
    for (unsigned i = 0; i < n; i++)
    {
        unsigned v = 0;
        CHECK(scanf(" %c %u", &op, &v) == 2);
        uint32_t out = 999;
        enum ts_status r;
        if (op == 'P')
        {
            r = ts_push(&s, v);
        }
        else if (op == 'O')
        {
            r = ts_pop(&s, 0, &out);
        }
        else
        {
            out = ts_collect(&s);
            r = TS_OK;
        }
        printf("%u %u\n", (unsigned)r, out);
    }
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "driver"))
    {
        driver();
        return 0;
    }
    if (!strcmp(argv[1], "history"))
    {
        history();
        return 0;
    }
    if (!strcmp(argv[1], "stack"))
    {
        stack();
        native();
    }
    else
    {
        CHECK(!strcmp(argv[1], "reclaim"));
        reclaim();
    }
    printf("TS_PASS gen=%u stack=%zu node=%zu pop=%zu\n", TS_GEN_MAX,
           sizeof(struct ts_stack), sizeof(struct ts_node),
           sizeof(struct ts_pop));
    return 0;
}
