#define _POSIX_C_SOURCE 200809L
#include "progress.h"
#include "queue.h"
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "TP_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            exit(41);                                                        \
        }                                                                    \
    } while (0)

struct tw_call
{
    struct tp_end_queue *q;
    int value, result;
};

static void *pop(void *p)
{
    struct tw_call *c = p;
    c->result = tp_queue_pop(c->q, &c->value);
    return NULL;
}

static void *push(void *p)
{
    struct tw_call *c = p;
    int e = tp_queue_push(c->q, c->value);
    pthread_testcancel();
    c->result = e;
    return NULL;
}

static void shutdown_case(void)
{
    struct tp_end_queue q;
    CHECK(tp_queue_init(&q) == 0);
    CHECK(tp_queue_push(&q, -1) == EINVAL &&
          tp_queue_push(&q, 1000001) == EINVAL);
    for (int i = 1; i <= 3; i++)
    {
        CHECK(tp_queue_push(&q, i) == 0);
    }
    struct tw_call c = {.q = &q, .value = 4};
    pthread_t t;
    CHECK(pthread_create(&t, NULL, push, &c) == 0);
    CHECK(tw_wait_entries(&q.queue, 1) == 0);
    CHECK(tp_queue_abort(&q) == 0);
    CHECK(pthread_join(t, NULL) == 0);
    CHECK(c.result == EPIPE);
    CHECK(q.accepted == 3 && q.delivered == 0 && q.discarded == 3 &&
          q.discarded_sum == 6 && q.retired == 1);
    int n = 77;
    CHECK(tp_queue_pop(&q, &n) == ECANCELED && n == 77);
    CHECK(tp_queue_push(&q, 5) == ECANCELED);
    CHECK(tp_queue_abort(&q) == EALREADY);
    tp_queue_destroy(&q);
    CHECK(tp_queue_init(&q) == 0);
    c = (struct tw_call){.q = &q, .value = 77};
    CHECK(pthread_create(&t, NULL, pop, &c) == 0);
    CHECK(tw_wait_entries(&q.queue, 1) == 0);
    CHECK(tp_queue_abort(&q) == 0);
    CHECK(pthread_join(t, NULL) == 0);
    CHECK(c.result == EPIPE && c.value == 77 && q.accepted == 0 &&
          q.delivered == 0 && q.discarded == 0);
    tp_queue_destroy(&q);
    CHECK(tp_queue_init(&q) == 0);
    for (int i = 1; i <= 3; i++)
    {
        CHECK(tp_queue_push(&q, i) == 0);
    }
    c = (struct tw_call){.q = &q, .value = 4, .result = 99};
    CHECK(pthread_create(&t, NULL, push, &c) == 0);
    CHECK(tw_wait_entries(&q.queue, 1) == 0);
    CHECK(pthread_cancel(t) == 0);
    CHECK(tp_queue_abort(&q) == 0);
    void *terminal = NULL;
    CHECK(pthread_join(t, &terminal) == 0 && terminal == PTHREAD_CANCELED);
    CHECK(q.inflight == 0 && q.accepted == 3 && q.discarded == 3 &&
          c.result == 99);
    tp_queue_destroy(&q);
    puts(
        "OBS shutdown admitted blocked producer/consumer settle; residual copied "
        "values discarded once; no new admissions; destroy only after join");
}

struct tw_stress
{
    struct tw_queue *q;
    unsigned id;
    unsigned seen[2][1000];
};

struct history_call
{
    struct tw_queue *queue;
    pthread_mutex_t *clock_lock;
    unsigned *clock;
    struct tp_op *record;
};

static void *history_call(void *arg)
{
    struct history_call *h = arg;
    CHECK(pthread_mutex_lock(h->clock_lock) == 0);
    h->record->invocation = ++*h->clock;
    CHECK(pthread_mutex_unlock(h->clock_lock) == 0);
    int value = (int)h->record->value;
    int e = h->record->kind == TP_ENQUEUE
                ? tw_push(h->queue, value, NULL)
                : tw_pop(h->queue, &value, NULL, NULL, NULL);
    CHECK(e == 0);
    CHECK(pthread_mutex_lock(h->clock_lock) == 0);
    h->record->response = ++*h->clock;
    h->record->result = TP_VALUE;
    h->record->value = (unsigned)value;
    CHECK(pthread_mutex_unlock(h->clock_lock) == 0);
    return NULL;
}

static void actual_histories(void)
{
    for (unsigned repeat = 0; repeat < 20; repeat++)
    {
        struct tw_queue q;
        struct tw_init init;
        CHECK(tw_queue_init(&q, 0, &init) == 0);
        pthread_mutex_t clock_lock;
        CHECK(pthread_mutex_init(&clock_lock, NULL) == 0);
        unsigned clock = 0;
        struct tp_op records[4] = {
            {.thread = 0, .kind = TP_DEQUEUE},
            {.thread = 1, .kind = TP_DEQUEUE},
            {.thread = 2, .kind = TP_ENQUEUE, .value = 1},
            {.thread = 3, .kind = TP_ENQUEUE, .value = 2}};
        struct history_call calls[4];
        pthread_t threads[4];
        for (unsigned i = 0; i < 4; i++)
        {
            calls[i] =
                (struct history_call){&q, &clock_lock, &clock, &records[i]};
            if (i == 2)
            {
                CHECK(tw_wait_entries(&q, 2) == 0);
            }
            CHECK(pthread_create(&threads[i], NULL, history_call, &calls[i]) ==
                  0);
        }
        for (unsigned i = 0; i < 4; i++)
        {
            CHECK(pthread_join(threads[i], NULL) == 0);
        }
        struct tp_witness witness;
        CHECK(clock == 8 &&
              tp_history(records, 4, TP_REQUIRE_COMPLETE, &witness) == TP_OK);
        CHECK(tw_close(&q) == 0 && tw_unref(&q) == 0 && tw_retire(&q) == 0);
        tw_root_destroy(&q);
        CHECK(pthread_mutex_destroy(&clock_lock) == 0);
    }
}

static void *producer(void *arg)
{
    struct tw_stress *s = arg;
    uint32_t seed = UINT32_C(0x12345678) + s->id;
    for (unsigned i = 0; i < 1000; i++)
    {
        seed = seed * 1664525u + 1013904223u;
        if (seed & 1u)
        {
            sched_yield();
        }
        CHECK(tw_push(s->q, (int)(s->id * 1000 + i), NULL) == 0);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    struct tw_stress *s = arg;
    for (;;)
    {
        int value = 0;
        int e = tw_pop(s->q, &value, NULL, NULL, NULL);
        if (e == EPIPE)
        {
            break;
        }
        CHECK(e == 0 && value >= 0 && value < 2000);
        s->seen[(unsigned)value / 1000][(unsigned)value % 1000]++;
    }
    return NULL;
}

static void stress(void)
{
    struct tw_queue q;
    struct tw_init init;
    CHECK(tw_queue_init(&q, 0, &init) == 0);
    CHECK(tw_push(&q, 7, NULL) == 0);
    int probe = 0;
    CHECK(tw_pop(&q, &probe, NULL, NULL, NULL) == 0 && probe == 7);
    struct tw_stress p[2] = {{.q = &q, .id = 0}, {.q = &q, .id = 1}},
                     c[2] = {{.q = &q}, {.q = &q}};
    pthread_t pt[2], ct[2];
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(pthread_create(&ct[i], NULL, consumer, &c[i]) == 0);
    }
    CHECK(tw_wait_entries(&q, 2) == 0);
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(pthread_create(&pt[i], NULL, producer, &p[i]) == 0);
    }
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(pthread_join(pt[i], NULL) == 0);
    }
    CHECK(tw_close(&q) == 0);
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(pthread_join(ct[i], NULL) == 0);
    }
    for (unsigned producer_id = 0; producer_id < 2; producer_id++)
    {
        for (unsigned i = 0; i < 1000; i++)
        {
            CHECK(c[0].seen[producer_id][i] + c[1].seen[producer_id][i] == 1);
        }
    }
    CHECK(tw_unref(&q) == 0);
    CHECK(tw_retire(&q) == 0);
    tw_root_destroy(&q);
    actual_histories();
    puts("OBS stress frozen capacity3 queue,2000 unique payloads,2 producers/2 "
         "consumers,initial forced enrollment,fixed per-producer seeds,exact "
         "once accounting");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "shutdown"))
    {
        shutdown_case();
    }
    else if (!strcmp(argv[1], "stress"))
    {
        stress();
    }
    else
    {
        return 2;
    }
    return 0;
}
