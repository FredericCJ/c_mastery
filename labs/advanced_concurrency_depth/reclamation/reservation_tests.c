#define _POSIX_C_SOURCE 200809L
#include "cmx_advanced.h"
#include "reservation.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "AR_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(41);                                           \
        }                                                       \
    } while (0)

struct ag_gate
{
    pthread_mutex_t mutex;
    pthread_cond_t cv;
    unsigned reached, go;
};

static void hook(void *ctx, unsigned point, unsigned p, unsigned ticket)
{
    (void)point;
    (void)p;
    if (ticket)
        return;
    struct ag_gate *g = ctx;
    CHECK(!pthread_mutex_lock(&g->mutex));
    g->reached = 1;
    CHECK(!pthread_cond_broadcast(&g->cv));
    while (!g->go)
        CHECK(!pthread_cond_wait(&g->cv, &g->mutex));
    CHECK(!pthread_mutex_unlock(&g->mutex));
}

struct ag_work
{
    struct ar_queue *q;
    struct ar_ticket ticket;
    int result;
};

static void *finish(void *ptr)
{
    struct ag_work *w = ptr;
    w->result = ar_finish(w->q, w->ticket, (struct ap_value){11, ~11u});
    return NULL;
}

int main(void)
{
    cmx_reservation retained;
    uint32_t a, b, out = 99;
    cmx_reservation_init(&retained);
    CHECK(cmx_reserve(&retained, &a) == CMX_OK &&
          cmx_reserve(&retained, &b) == CMX_OK);
    CHECK(cmx_publish(&retained, b, 22) == CMX_OK);
    CHECK(cmx_reserved_pop(&retained, &out) == CMX_RETRY && out == 99);
    CHECK(cmx_publish(&retained, a, 11) == CMX_OK);
    CHECK(cmx_reserved_pop(&retained, &out) == CMX_OK && out == 11);
    CHECK(cmx_reserved_pop(&retained, &out) == CMX_OK && out == 22);
    struct ag_gate g = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0,
                        0};
    struct ar_queue q;
    ar_init(&q, hook, &g);
    struct ar_ticket first, second;
    struct ap_value v = {99, ~99u};
    CHECK(ar_pop(&q, &v) == AR_EMPTY && v.sequence == 99);
    CHECK(ar_reserve(&q, &first) == AR_OK && ar_reserve(&q, &second) == AR_OK);
    CHECK(ar_finish(&q, second, (struct ap_value){22, ~22u}) == AR_OK);
    CHECK(ar_pop(&q, &v) == AR_PENDING && v.sequence == 99);
    struct ag_work w = {&q, first, AR_UNIMPLEMENTED};
    pthread_t worker;
    CHECK(!pthread_create(&worker, NULL, finish, &w));
    CHECK(!pthread_mutex_lock(&g.mutex));
    while (!g.reached)
        CHECK(!pthread_cond_wait(&g.cv, &g.mutex));
    CHECK(!pthread_mutex_unlock(&g.mutex));
    CHECK(ar_cancel(&q, first) == AR_OK);
    int waiting = ar_pop(&q, &v);
    CHECK(!pthread_mutex_lock(&g.mutex));
    g.go = 1;
    CHECK(!pthread_cond_broadcast(&g.cv));
    CHECK(!pthread_mutex_unlock(&g.mutex));
    CHECK(!pthread_join(worker, NULL));
    CHECK(waiting == AR_PENDING && v.sequence == 99);
    CHECK(w.result == AR_CANCELLED);
    CHECK(ar_pop(&q, &v) == AR_CANCELLED && v.sequence == 99);
    CHECK(ar_cancel(&q, second) == AR_TOO_LATE);
    CHECK(ar_pop(&q, &v) == AR_OK && v.sequence == 22 && v.inverse == ~22u);
    CHECK(ar_cancel(&q, first) == AR_INVALID);
    CHECK(ar_pop(&q, &v) == AR_EMPTY);
    q.hook = NULL;
    for (unsigned i = 0; i < 32; i++)
    {
        struct ar_ticket t;
        CHECK(ar_reserve(&q, &t) == AR_OK);
        CHECK(ar_finish(&q, t, (struct ap_value){i, ~i}) == AR_OK);
        CHECK(ar_pop(&q, &v) == AR_OK && v.sequence == i);
        CHECK(ar_cancel(&q, first) == AR_INVALID);
    }
    struct ar_ticket tickets[4], unchanged = first;
    for (unsigned i = 0; i < 4; i++)
        CHECK(ar_reserve(&q, &tickets[i]) == AR_OK);
    CHECK(ar_reserve(&q, &unchanged) == AR_FULL &&
          unchanged.sequence == first.sequence);
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(ar_cancel(&q, tickets[i]) == AR_OK);
        CHECK(ar_finish(&q, tickets[i], (struct ap_value){0, 0}) ==
              AR_CANCELLED);
        CHECK(ar_pop(&q, &v) == AR_CANCELLED);
    }
    atomic_store(&q.head, AR_LIMIT);
    atomic_store(&q.tail, AR_LIMIT);
    CHECK(ar_reserve(&q, &unchanged) == AR_EXHAUSTED &&
          unchanged.sequence == first.sequence);
    CHECK(ar_pop(&q, &v) == AR_EMPTY);
    CHECK(!pthread_cond_destroy(&g.cv));
    CHECK(!pthread_mutex_destroy(&g.mutex));
    puts("OBS reservation retained-pending-prefix owner-ack-before-reuse "
         "cancelled-skip stale-ticket exhaustion");
    return 0;
}
