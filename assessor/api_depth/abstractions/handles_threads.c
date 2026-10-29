#include "handles.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define REQUIRE(x)                                                        \
    do                                                                    \
    {                                                                     \
        if (!(x))                                                         \
        {                                                                 \
            fprintf(stderr, "HANDLE-THREAD-CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                                      \
        }                                                                 \
    } while (0)

struct ac_promotion
{
    struct ah_shared *shared;
    atomic_uint go;
    unsigned promoted;
};

static void *promote(void *arg)
{
    struct ac_promotion *p = arg;
    while (!atomic_load_explicit(&p->go, memory_order_acquire))
    {
    }
    enum ah_result r = ah_promote(p->shared);
    REQUIRE(r == AH_OK || r == AH_DEAD);
    p->promoted = r == AH_OK;
    if (r == AH_OK)
    {
        REQUIRE(*ah_payload(p->shared) == 37);
        REQUIRE(ah_drop(p->shared) == AH_OK);
    }
    return 0;
}

struct ac_worker
{
    struct ah_shared *shared;
    atomic_uint *go;
};

static void *write_payload(void *arg)
{
    struct ac_worker *w = arg;
    while (!atomic_load_explicit(w->go, memory_order_acquire))
    {
    }
    for (unsigned i = 0; i < 10000; ++i)
    {
#ifdef AH_RACY_PAYLOAD
        ++*ah_payload(w->shared);
#else
        REQUIRE(ah_add(w->shared, 1) == AH_OK);
#endif
    }
    REQUIRE(ah_drop(w->shared) == AH_OK);
    return 0;
}

struct ac_snapshot
{
    _Atomic(struct ah_shared *) published;
    unsigned observed;
};

static void *snapshot(void *arg)
{
    struct ac_snapshot *s = arg;
    struct ah_shared *p;
    do
    {
        p = atomic_load_explicit(&s->published, memory_order_acquire);
    } while (!p);
    s->observed = *ah_payload(p);
    REQUIRE(ah_drop(p) == AH_OK);
    return 0;
}

int main(void)
{
    struct ah_stats stats = {0};
    unsigned successes = 0;
    for (unsigned i = 0; i < 256; ++i)
    {
        struct ah_shared *p = 0;
        REQUIRE(ah_shared_create(37, &stats, &p) == AH_OK);
        REQUIRE(ah_weak_retain(p) == AH_OK);
        struct ac_promotion op = {p, 0, 0};
        atomic_init(&op.go, 0);
        pthread_t thread;
        REQUIRE(!pthread_create(&thread, 0, promote, &op));
        atomic_store_explicit(&op.go, 1, memory_order_release);
        REQUIRE(ah_drop(p) == AH_OK);
        REQUIRE(!pthread_join(thread, 0));
        successes += op.promoted;
        unsigned strong, weak;
        REQUIRE(ah_counts(p, &strong, &weak) == AH_OK && strong == 0 &&
                weak == 1);
        REQUIRE(ah_weak_drop(p) == AH_OK);
    }
    struct ah_shared *p = 0;
    REQUIRE(ah_shared_create(0, &stats, &p) == AH_OK);
    REQUIRE(ah_retain(p) == AH_OK && ah_retain(p) == AH_OK);
    atomic_uint go = 0;
    struct ac_worker worker = {p, &go};
    pthread_t a, b;
    REQUIRE(!pthread_create(&a, 0, write_payload, &worker));
    REQUIRE(!pthread_create(&b, 0, write_payload, &worker));
    atomic_store_explicit(&go, 1, memory_order_release);
    REQUIRE(!pthread_join(a, 0) && !pthread_join(b, 0));
#ifndef AH_RACY_PAYLOAD
    REQUIRE(*ah_payload(p) == 20000);
#endif
    REQUIRE(ah_drop(p) == AH_OK);
    for (unsigned i = 0; i < 32; ++i)
    {
        REQUIRE(ah_shared_create(100 + i, &stats, &p) == AH_OK);
        REQUIRE(ah_retain(p) == AH_OK);
        struct ac_snapshot s = {0};
        atomic_init(&s.published, 0);
        REQUIRE(!pthread_create(&a, 0, snapshot, &s));
        atomic_store_explicit(&s.published, p, memory_order_release);
        REQUIRE(!pthread_join(a, 0));
        REQUIRE(s.observed == 100 + i);
        REQUIRE(ah_drop(p) == AH_OK);
    }
    REQUIRE(atomic_load(&stats.controls) == 289 &&
            atomic_load(&stats.control_frees) == 289 &&
            atomic_load(&stats.payload_frees) == 289);
    printf(
        "HANDLE-THREAD-PASS:promotion_races=256;promoted=%u;payload_increments=20000;snapshots=32;balanced=289;\n",
        successes);
    return 0;
}
