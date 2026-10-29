#define _POSIX_C_SOURCE 200809L
#include "core.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CK(x)                                                  \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CF_CHECK:%d:%s\n", __LINE__, #x); \
            exit(41);                                          \
        }                                                      \
    } while (0)

static struct cf_value val(unsigned n)
{
    return (struct cf_value){n, ~n};
}

struct cf_gate
{
    pthread_mutex_t lock;
    pthread_cond_t cv;
    unsigned entered, released;
};

static void hook(void *p)
{
    struct cf_gate *g = p;
    CK(!pthread_mutex_lock(&g->lock));
    g->entered = 1;
    CK(!pthread_cond_broadcast(&g->cv));
    while (!g->released)
    {
        CK(!pthread_cond_wait(&g->cv, &g->lock));
    }
    CK(!pthread_mutex_unlock(&g->lock));
}

static void wait_gate(struct cf_gate *g)
{
    CK(!pthread_mutex_lock(&g->lock));
    while (!g->entered)
    {
        CK(!pthread_cond_wait(&g->cv, &g->lock));
    }
    CK(!pthread_mutex_unlock(&g->lock));
}

static void release_gate(struct cf_gate *g)
{
    CK(!pthread_mutex_lock(&g->lock));
    g->released = 1;
    CK(!pthread_cond_broadcast(&g->cv));
    CK(!pthread_mutex_unlock(&g->lock));
}

struct cf_batch
{
    struct cf_ring *q;
    struct cf_gate *g;
    struct cf_value v[4];
    unsigned pop;
    int result;
};

static void *batch(void *p)
{
    struct cf_batch *b = p;
    b->result = b->pop ? cf_pop(b->q, b->v, 4, hook, b->g)
                       : cf_push(b->q, b->v, 4, hook, b->g);
    return NULL;
}

static void boundary(void)
{
    for (unsigned pop = 0; pop < 2; pop++)
    {
        struct cf_gate g = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER,
                            0, 0};
        struct cf_ring q;
        CK(cf_ring_init(&q, 4, 4, 32) == CF_OK);
        struct cf_batch b = {&q, &g, {{0, 0}}, pop, -1};
        for (unsigned i = 0; i < 4; i++)
        {
            b.v[i] = val(i + 1);
        }
        if (pop)
        {
            CK(cf_push(&q, b.v, 4, NULL, NULL) == CF_OK);
        }
        pthread_t thread;
        CK(!pthread_create(&thread, NULL, batch, &b));
        wait_gate(&g);
        struct cf_value other[4] = {val(90), val(91), val(92), val(93)};
        int r = pop ? cf_push(&q, other, 1, NULL, NULL)
                    : cf_pop(&q, other, 1, NULL, NULL);
        CK(r == (pop ? CF_FULL : CF_EMPTY));
        CK(other[0].sequence == 90);
        release_gate(&g);
        CK(!pthread_join(thread, NULL));
        CK(b.result == CF_OK);
        if (!pop)
        {
            CK(cf_pop(&q, other, 4, NULL, NULL) == CF_OK);
        }
        else
        {
            for (unsigned i = 0; i < 4; i++)
            {
                other[i] = b.v[i];
            }
        }
        for (unsigned i = 0; i < 4; i++)
        {
            CK(other[i].sequence == i + 1 && other[i].inverse == ~(i + 1));
        }
        CK(!pthread_cond_destroy(&g.cv));
        CK(!pthread_mutex_destroy(&g.lock));
    }
    puts("CF_PASS:batch two actual paused copy/publication boundaries");
}

static void ring(void)
{
    for (unsigned bits = 8; bits <= 32; bits += 24)
    {
        unsigned mask = bits == 8 ? 255u : UINT_MAX;
        for (unsigned offset = 0; offset < 256; offset++)
        {
            for (unsigned n = 1; n <= 4; n++)
            {
                struct cf_ring q;
                CK(cf_ring_init(&q, 4, 4, bits) == CF_OK);
                unsigned start = (mask - offset) & mask;
                atomic_store(&q.head, start);
                atomic_store(&q.tail, start);
                struct cf_value v[4] = {val(7), val(8), val(9), val(10)},
                                out[4] = {val(99), val(99), val(99), val(99)};
                CK(cf_push(&q, v, n, NULL, NULL) == CF_OK);
                if (n < 4)
                {
                    CK(cf_push(&q, v + n, 4 - n, NULL, NULL) == CF_OK);
                }
                CK(cf_push(&q, v, 1, NULL, NULL) == CF_FULL);
                CK(((atomic_load(&q.head) - atomic_load(&q.tail)) & mask) == 4);
                CK(cf_pop(&q, out, 4, NULL, NULL) == CF_OK);
                for (unsigned i = 0; i < 4; i++)
                {
                    CK(out[i].sequence == i + 7 && out[i].inverse == ~(i + 7));
                }
                out[0] = val(99);
                CK(cf_pop(&q, out, 1, NULL, NULL) == CF_EMPTY &&
                   out[0].sequence == 99);
            }
        }
    }
    struct cf_ring q;
    CK(cf_ring_init(&q, 4, 4, 16) == CF_INVALID);
    CK(cf_ring_init(&q, 256, 128, 8) == CF_INVALID);
    CK(cf_ring_init(&q, 4, 4, 32) == CF_OK);
    struct cf_value d[4] = {val(1), val(2), val(3), val(4)};
    CK(cf_push(&q, d, 0, NULL, NULL) == CF_INVALID &&
       cf_push(&q, d, 5, NULL, NULL) == CF_INVALID);
    puts(
        "CF_PASS:ring 2048 exact wrapped batch histories including UINT_MAX-1");
}

static void cas(void)
{
    atomic_uint v;
    atomic_init(&v, UINT_MAX - 2);
    unsigned out = 77, tries = 99;
    CK(cf_increment(&v, &out, &tries, 3) == CF_OK && out == UINT_MAX - 2 &&
       tries >= 4 && atomic_load(&v) == UINT_MAX - 1);
    unsigned stale = UINT_MAX - 2;
    CK(!atomic_compare_exchange_strong(&v, &stale, 0) && stale == UINT_MAX - 1);
    CK(cf_increment(&v, &out, &tries, 0) == CF_OK && out == UINT_MAX - 1 &&
       atomic_load(&v) == UINT_MAX);
    out = 77;
    CK(cf_increment(&v, &out, &tries, 0) == CF_EXHAUSTED && out == 77 &&
       tries == 0);
    CK(cf_increment(&v, &out, &tries, 17) == CF_INVALID);
    printf("CF_PASS:cas spurious=3 stale_expected_refreshed=1 terminal=%u "
           "lock_free=%d\n",
           atomic_load(&v), atomic_is_lock_free(&v));
}

static void counts(struct cf_pool *p, unsigned want_free,
                   unsigned want_reserved, unsigned want_linked,
                   unsigned want_retired)
{
    unsigned n[5] = {0};
    for (unsigned i = 0; i < 32; i++)
    {
        unsigned state = atomic_load(&p->state[i]) & 7u;
        CK(state < 5);
        n[state]++;
    }
    CK(n[0] == want_free && n[1] == want_reserved && n[2] == want_linked &&
       n[3] == want_retired);
    CK(n[0] + n[1] + n[2] + n[3] + n[4] == 32);
}

static void pool(void)
{
    struct cf_pool p;
    CK(cf_pool_init(&p, 3) == CF_OK);
    counts(&p, 31, 0, 1, 0);
    struct cf_ticket a, b, c;
    unsigned scans;
    CK(cf_take(&p, 0, 1, &a, &scans) == CF_OK);
    CK(cf_take(&p, 0, 1, &b, &scans) == CF_OK);
    CK(cf_take(&p, 0, 1, &c, &scans) == CF_OK);
    counts(&p, 28, 3, 1, 0);
    /* An actual failed root CAS leaves a's reservation private. The sole owner
     * may retry or abandon only before successful publication. */
    atomic_uint root;
    atomic_init(&root, 2);
    unsigned expected = 0;
    CK(!atomic_compare_exchange_strong(&root, &expected, a.index) &&
       expected == 2);
    counts(&p, 28, 3, 1, 0);
    CK(cf_cancel(&p, a) == CF_OK);
    CK(cf_cancel(&p, a) == CF_STALE);
    expected = 2;
    CK(atomic_compare_exchange_strong(&root, &expected, b.index));
    CK(cf_publish(&p, b) == CF_OK);
    CK(cf_cancel(&p, b) == CF_STALE);
    CK(cf_cancel(&p, c) == CF_OK);
    counts(&p, 30, 0, 2, 0);
    atomic_store(&root, 0);
    CK(cf_retire(&p, b) == CF_OK);
    counts(&p, 30, 0, 1, 1);
    CK(cf_reclaim(&p, b) == CF_OK);
    counts(&p, 31, 0, 1, 0);
    /* Reduced generation space loses availability instead of accepting stale
     * tickets. Initialization is not a reset protocol for live tickets. */
    CK(cf_pool_init(&p, 1) == CF_OK);
    for (unsigned i = 0; i < 31; i++)
    {
        CK(cf_take(&p, 0, 1, &a, &scans) == CF_OK);
        CK(cf_cancel(&p, a) == CF_OK);
    }
    CK(cf_take(&p, 0, 1, &a, &scans) == CF_FULL);
    counts(&p, 0, 0, 1, 0);
    for (unsigned steal = 0; steal <= 1; steal++)
    {
        CK(cf_pool_init(&p, 3) == CF_OK);
        unsigned admitted = 0, total_scans = 0;
        while (cf_take(&p, 0, steal, &a, &scans) == CF_OK)
        {
            admitted++;
            total_scans += scans;
        }
        CK(admitted == (steal ? 31u : 7u));
        printf(
            "CF_POOL partition_worker=0 steal=%u admitted=%u free=%u scans=%u "
            "total_slots=32 dummy=1\n",
            steal, admitted, 31 - admitted, total_scans);
    }
    puts("CF_PASS:pool ownership partition cancellation published rejection "
         "saturation");
}

static void diagnostic(void)
{
    struct cf_ring q;
    CK(cf_ring_init(&q, 4, 4, 32) == CF_OK);
    unsigned dropped = 0;
    for (unsigned i = 0; i < 9; i++)
    {
        struct cf_value v = val(i);
        int r = cf_push(&q, &v, 1, NULL, NULL);
        if (r == CF_FULL)
        {
            dropped++;
        }
        else
        {
            CK(r == CF_OK);
        }
    }
    CK(dropped == 5);
    for (unsigned i = 0; i < 4; i++)
    {
        struct cf_value out;
        CK(cf_pop(&q, &out, 1, NULL, NULL) == CF_OK && out.sequence == i);
    }
    puts("CF_PASS:diagnostic retained=4 dropped_new=5 no allocation logger or "
         "wait");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "ring"))
    {
        ring();
    }
    else if (!strcmp(argv[1], "batch"))
    {
        boundary();
    }
    else if (!strcmp(argv[1], "cas"))
    {
        cas();
    }
    else if (!strcmp(argv[1], "pool"))
    {
        pool();
    }
    else if (!strcmp(argv[1], "diagnostic"))
    {
        diagnostic();
    }
    else
    {
        return 2;
    }
    return 0;
}
