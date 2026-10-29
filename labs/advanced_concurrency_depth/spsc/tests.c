#define _GNU_SOURCE
#include "spsc.h"
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "AS_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(41);                                           \
        }                                                       \
    } while (0)

static struct as_value value(unsigned n)
{
    return (struct as_value){n, ~(uint32_t)n, n ^ UINT32_C(0x91a337e2),
                             n * UINT32_C(3)};
}

static void matches(struct as_value v, unsigned n)
{
    struct as_value e = value(n);
    CHECK(v.sequence == e.sequence && v.inverse == e.inverse &&
          v.salt == e.salt && v.triple == e.triple);
}

static void interface(void)
{
    struct as_queue q;
    q.capacity = 77;
    for (unsigned n = 0; n <= 20; n++)
    {
        int valid = n && n <= 16 && !(n & (n - 1));
        enum cmx_result r = as_init(&q, n);
        CHECK(r == (valid ? CMX_OK : CMX_INVALID));
        if (!valid)
            continue;
        CHECK(q.capacity == n);
        struct as_value out = value(77);
        CHECK(as_pop(&q, &out, NULL, NULL) == CMX_RETRY);
        matches(out, 77);
        for (unsigned i = 0; i < n; i++)
            CHECK(as_push(&q, value(i), NULL, NULL) == CMX_OK);
        CHECK(as_push(&q, value(90), NULL, NULL) == CMX_FULL);
        for (unsigned i = 0; i < n; i++)
        {
            CHECK(as_pop(&q, &out, NULL, NULL) == CMX_OK);
            matches(out, i);
        }
        CHECK(as_pop(&q, &out, NULL, NULL) == CMX_RETRY);
    }
    CHECK(as_init(&q, 3) == CMX_INVALID && q.capacity == 16);
    CHECK(as_pop(&q, NULL, NULL, NULL) == CMX_INVALID);
    printf("LAYOUT head=%zu tail=%zu align=%zu size=%zu line=%ld "
           "lockfree_head=%d lockfree_tail=%d\n",
           offsetof(struct as_queue, head), offsetof(struct as_queue, tail),
           _Alignof(struct as_queue), sizeof q,
           sysconf(_SC_LEVEL1_DCACHE_LINESIZE), atomic_is_lock_free(&q.head),
           atomic_is_lock_free(&q.tail));
    puts("OBS interface capacity admission,full/empty,multiword FIFO,output "
         "preservation,actual layout/lock freedom");
}

struct ag_gate
{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    unsigned reached, release;
    struct as_queue *q;
    struct as_value out;
    enum cmx_result result;
    unsigned pushing;
};

static void pause_at(void *p)
{
    struct ag_gate *g = p;
    CHECK(pthread_mutex_lock(&g->lock) == 0);
    g->reached = 1;
    CHECK(pthread_cond_broadcast(&g->cond) == 0);
    while (!g->release)
        CHECK(pthread_cond_wait(&g->cond, &g->lock) == 0);
    CHECK(pthread_mutex_unlock(&g->lock) == 0);
}

static void *paused(void *p)
{
    struct ag_gate *g = p;
    g->result = g->pushing ? as_push(g->q, value(42), pause_at, g)
                           : as_pop(g->q, &g->out, pause_at, g);
    return NULL;
}

static void gate_wait(struct ag_gate *g)
{
    CHECK(pthread_mutex_lock(&g->lock) == 0);
    while (!g->reached)
        CHECK(pthread_cond_wait(&g->cond, &g->lock) == 0);
    CHECK(pthread_mutex_unlock(&g->lock) == 0);
}

static void gate_release(struct ag_gate *g)
{
    CHECK(pthread_mutex_lock(&g->lock) == 0);
    g->release = 1;
    CHECK(pthread_cond_broadcast(&g->cond) == 0);
    CHECK(pthread_mutex_unlock(&g->lock) == 0);
}

struct ag_reentry
{
    struct as_queue *q;
    unsigned producer;
    enum cmx_result result;
};

static void reenter(void *p)
{
    struct ag_reentry *r = p;
    struct as_value out = value(77);
    r->result = r->producer ? as_push(r->q, value(99), NULL, NULL)
                            : as_pop(r->q, &out, NULL, NULL);
    if (!r->producer)
        matches(out, 77);
}

struct ag_worker
{
    struct as_queue *q;
    unsigned producer;
};

static void *transfer(void *p)
{
    struct ag_worker *w = p;
    for (unsigned i = 0; i < 4000; i++)
    {
        enum cmx_result r;
        struct as_value out = value(77);
        do
        {
            r = w->producer ? as_push(w->q, value(i), NULL, NULL)
                            : as_pop(w->q, &out, NULL, NULL);
            if (r == CMX_FULL || r == CMX_RETRY)
                sched_yield();
        } while (r == CMX_FULL || r == CMX_RETRY);
        CHECK(r == CMX_OK);
        if (!w->producer)
            matches(out, i);
    }
    return NULL;
}

static void edges(void)
{
    struct as_queue q;
    CHECK(as_init(&q, 1) == CMX_OK);
    struct ag_gate g = {.q = &q, .pushing = 1};
    CHECK(pthread_mutex_init(&g.lock, NULL) == 0 &&
          pthread_cond_init(&g.cond, NULL) == 0);
    pthread_t t;
    CHECK(pthread_create(&t, NULL, paused, &g) == 0);
    gate_wait(&g);
    struct as_value out = value(77);
    CHECK(as_pop(&q, &out, NULL, NULL) == CMX_RETRY);
    matches(out, 77);
    gate_release(&g);
    CHECK(pthread_join(t, NULL) == 0 && g.result == CMX_OK);
    g.reached = g.release = g.pushing = 0;
    CHECK(pthread_create(&t, NULL, paused, &g) == 0);
    gate_wait(&g);
    CHECK(as_push(&q, value(43), NULL, NULL) == CMX_FULL);
    gate_release(&g);
    CHECK(pthread_join(t, NULL) == 0 && g.result == CMX_OK);
    matches(g.out, 42);
    CHECK(pthread_cond_destroy(&g.cond) == 0 &&
          pthread_mutex_destroy(&g.lock) == 0);
    struct ag_reentry re = {.q = &q, .producer = 1};
    CHECK(as_push(&q, value(7), reenter, &re) == CMX_OK &&
          re.result == CMX_INVALID);
    re.producer = 0;
    CHECK(as_pop(&q, &out, reenter, &re) == CMX_OK && re.result == CMX_INVALID);
    matches(out, 7);
    const unsigned capacities[] = {1, 2, 4, 16};
    for (unsigned k = 0; k < 4; k++)
    {
        CHECK(as_init(&q, capacities[k]) == CMX_OK);
        atomic_store(&q.head, UINT_MAX - 2);
        atomic_store(&q.tail, UINT_MAX - 2);
        struct ag_worker w[2] = {{&q, 1}, {&q, 0}};
        pthread_t threads[2];
        CHECK(pthread_create(&threads[0], NULL, transfer, &w[0]) == 0 &&
              pthread_create(&threads[1], NULL, transfer, &w[1]) == 0);
        CHECK(pthread_join(threads[0], NULL) == 0 &&
              pthread_join(threads[1], NULL) == 0);
        CHECK(atomic_load(&q.head) == atomic_load(&q.tail));
    }
    puts("OBS edges paused publication and reuse,reentrant producer/consumer "
         "rejection,16000 multiword concurrent transfers over unsigned wrap");
}

static void wrap(void)
{
    const unsigned capacities[] = {1, 2, 4, 16};
    unsigned states = 0, edges = 0;
    struct as_queue q;
    for (unsigned k = 0; k < 4; k++)
    {
        unsigned cap = capacities[k];
        for (unsigned tail = 0; tail < 64; tail++)
            for (unsigned used = 0; used <= cap; used++)
            {
                unsigned absolute = UINT_MAX - 31u + tail;
                CHECK(as_init(&q, cap) == CMX_OK);
                atomic_store(&q.tail, absolute);
                atomic_store(&q.head, absolute + used);
                for (unsigned i = 0; i < used; i++)
                    q.data[(absolute + i) & (cap - 1)] = value(i);
                struct as_value out = value(77);
                CHECK(as_pop(&q, &out, NULL, NULL) ==
                      (used ? CMX_OK : CMX_RETRY));
                if (used)
                {
                    matches(out, 0);
                    edges++;
                }
                else
                    matches(out, 77);
                CHECK(as_init(&q, cap) == CMX_OK);
                atomic_store(&q.tail, absolute);
                atomic_store(&q.head, absolute + used);
                CHECK(as_push(&q, value(99), NULL, NULL) ==
                      (used < cap ? CMX_OK : CMX_FULL));
                if (used < cap)
                    edges++;
                states++;
            }
    }
    CHECK(states == 1728 && edges == 2944);
    printf("OBS wrap states=%u edges=%u six-bit projected distance "
           "corpus,capacities1/2/4/16,actual unsigned wrap\n",
           states, edges);
}

static void circular(void)
{
    struct as_circular q;
    struct as_value out;
    for (unsigned n = 1; n <= 16; n *= 2)
    {
        CHECK(as_circular_init(&q, n) == CMX_OK);
        for (unsigned cycle = 0; cycle < 100; cycle++)
        {
            for (unsigned i = 0; i < n - 1; i++)
                CHECK(as_circular_push(&q, value(i)) == CMX_OK);
            CHECK(as_circular_push(&q, value(99)) == CMX_FULL);
            for (unsigned i = 0; i < n - 1; i++)
            {
                CHECK(as_circular_pop(&q, &out) == CMX_OK);
                matches(out, i);
            }
            out = value(77);
            CHECK(as_circular_pop(&q, &out) == CMX_RETRY);
            matches(out, 77);
        }
    }
    CHECK(as_circular_init(&q, 3) == CMX_INVALID);
    puts("OBS circular N-1 capacity including N1 zero-capacity,empty/full/wrap "
         "changed rules");
}

static void retained(void)
{
    cmx_spsc q;
    CHECK(!cmx_spsc_init(&q, 0) && !cmx_spsc_init(&q, 3) &&
          !cmx_spsc_init(&q, 17));
    const unsigned caps[] = {1, 2, 4, 16};
    for (unsigned k = 0; k < 4; k++)
    {
        unsigned n = caps[k];
        CHECK(cmx_spsc_init(&q, n));
        atomic_store(&q.head, UINT_MAX - 2);
        atomic_store(&q.tail, UINT_MAX - 2);
        for (unsigned i = 0; i < n; i++)
            CHECK(cmx_spsc_push(&q, i) == CMX_OK);
        CHECK(cmx_spsc_push(&q, 99) == CMX_FULL);
        uint32_t v;
        for (unsigned i = 0; i < n; i++)
        {
            CHECK(cmx_spsc_pop(&q, &v) == CMX_OK && v == i);
        }
        v = 77;
        CHECK(cmx_spsc_pop(&q, &v) == CMX_RETRY && v == 77);
    }
    puts("OBS retained actual CMX scalar artifact,boolean initialization "
         "contract preserved");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "interface"))
        interface();
    else if (!strcmp(argv[1], "edges"))
        edges();
    else if (!strcmp(argv[1], "wrap"))
        wrap();
    else if (!strcmp(argv[1], "circular"))
        circular();
    else if (!strcmp(argv[1], "retained"))
        retained();
    else
        return 2;
    return 0;
}
