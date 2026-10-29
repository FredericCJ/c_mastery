#define _POSIX_C_SOURCE 200809L
#include "cl_posix.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CL_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

struct cl_delivery
{
    uint32_t producer, sequence;
};

struct cl_worker
{
    struct clp_queue *queue;
    unsigned id;
    struct cl_delivery *trace;
    int result;
};

static void *producer(void *arg)
{
    struct cl_worker *w = arg;
    for (unsigned i = 0; i < 500; i++)
    {
        unsigned char b[4] = {(unsigned char)w->id, (unsigned char)i,
                              (unsigned char)(i >> 8), 0};
        CHECK(clp_send(w->queue, w->id, i, b, 4, 0) == 0);
        b[0] = 255;
    }
    return 0;
}

static void *consumer(void *arg)
{
    struct cl_worker *w = arg;
    for (;;)
    {
        unsigned char b[4];
        size_t n = 99;
        uint32_t p = 99, s = 99;
        uint64_t ticket = 0;
        int r = clp_recv(w->queue, b, 4, &n, &p, &s, &ticket, 0);
        if (r == EPIPE)
            break;
        CHECK(r == 0 && n == 4 && p < 3 && s < 500 && ticket >= 1 &&
              ticket <= 1500);
        CHECK(b[0] == p && b[1] == (unsigned char)s &&
              b[2] == (unsigned char)(s >> 8) && b[3] == 0);
        w->trace[ticket - 1] = (struct cl_delivery){p, s};
    }
    return 0;
}

static void *one_sender(void *arg);
static void enrolled(struct clp_queue *q, unsigned send);

static void backpressure_publication(void)
{
    /* Force a completed blocked send before the stress workload. A wakeup-only
     * mutant otherwise may strand later producers instead of exposing its
     * missing publication through the semantic assertion. */
    struct clp_queue q;
    CHECK(clp_init(&q, 0) == 0);
    unsigned char b = 1;
    CHECK(clp_send(&q, 0, 1, &b, 1, 0) == 0);
    CHECK(clp_send(&q, 0, 2, &b, 1, 0) == 0);
    struct cl_worker w = {&q, 0, 0, -1};
    pthread_t thread;
    CHECK(pthread_create(&thread, 0, one_sender, &w) == 0);
    enrolled(&q, 1);
    size_t n;
    uint32_t p, s;
    uint64_t ticket;
    CHECK(clp_recv(&q, &b, 1, &n, &p, &s, &ticket, 0) == 0 && s == 1);
    CHECK(pthread_join(thread, 0) == 0 && w.result == 0 &&
          q.core.accepted == 3);
    CHECK(clp_recv(&q, &b, 1, &n, &p, &s, &ticket, 0) == 0 && s == 2);
    CHECK(clp_recv(&q, &b, 1, &n, &p, &s, &ticket, 0) == 0 && s == 3 && b == 3);
    CHECK(clp_close(&q, 0) == 0 && clp_destroy(&q) == 0);
}

static void threaded(void)
{
    backpressure_publication();
    struct clp_queue q;
    CHECK(clp_init(&q, 0) == 0);
    struct cl_delivery trace[1500];
    for (unsigned i = 0; i < 1500; i++)
        trace[i] = (struct cl_delivery){99, 99};
    struct cl_worker w[5];
    pthread_t threads[5];
    for (unsigned i = 0; i < 5; i++)
    {
        w[i] = (struct cl_worker){&q, i, trace, -1};
        CHECK(pthread_create(&threads[i], 0, i < 3 ? producer : consumer,
                             &w[i]) == 0);
    }
    for (unsigned i = 0; i < 3; i++)
        CHECK(pthread_join(threads[i], 0) == 0);
    CHECK(clp_close(&q, 0) == 0);
    for (unsigned i = 3; i < 5; i++)
        CHECK(pthread_join(threads[i], 0) == 0);
    unsigned next[3] = {0};
    for (unsigned i = 0; i < 1500; i++)
    {
        CHECK(trace[i].producer < 3);
        CHECK(trace[i].sequence == next[trace[i].producer]++);
    }
    CHECK(next[0] == 500 && next[1] == 500 && next[2] == 500);
    CHECK(q.core.accepted == 1500 && q.core.delivered == 1500 &&
          q.core.discarded == 0 && q.active == 0);
    CHECK(clp_destroy(&q) == 0);
    puts("CL_OBSERVATION producers=3 consumers=2 accepted=1500 delivered=1500 "
         "ordered=1500");
}

static void *one_sender(void *arg)
{
    struct cl_worker *w = arg;
    unsigned char b = 3;
    w->result = clp_send(w->queue, 0, 3, &b, 1, 0);
    return 0;
}

static void *one_receiver(void *arg)
{
    struct cl_worker *w = arg;
    unsigned char b;
    size_t n = 99;
    uint32_t p, s;
    uint64_t ticket;
    w->result = clp_recv(w->queue, &b, 1, &n, &p, &s, &ticket, 0);
    CHECK(w->result != EPIPE || n == 99);
    return 0;
}

static void enrolled(struct clp_queue *q, unsigned send)
{
    struct timespec deadline;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &deadline) == 0);
    deadline.tv_sec += 5;
    CHECK(pthread_mutex_lock(&q->mutex) == 0);
    while ((send ? q->send_waiters : q->recv_waiters) == 0)
        CHECK(pthread_cond_timedwait(&q->enrolled, &q->mutex, &deadline) == 0);
    CHECK(pthread_mutex_unlock(&q->mutex) == 0);
}

struct cl_timer
{
    pthread_mutex_t lock;
    pthread_cond_t condition;
    unsigned entered, stop;
};

static void *timer(void *arg)
{
    struct cl_timer *t = arg;
    CHECK(pthread_mutex_lock(&t->lock) == 0);
    t->entered = 1;
    CHECK(pthread_cond_broadcast(&t->condition) == 0);
    while (!t->stop)
        CHECK(pthread_cond_wait(&t->condition, &t->lock) == 0);
    CHECK(pthread_mutex_unlock(&t->lock) == 0);
    return 0;
}

static void waiting(void)
{
    struct clp_queue q;
    for (unsigned i = 1; i <= 4; i++)
        CHECK(clp_init(&q, i) == EAGAIN && q.initialized == 0);
    CHECK(clp_init(&q, 0) == 0 && clp_destroy(&q) == EBUSY);
    unsigned char b = 1;
    CHECK(clp_send(&q, 0, 1, &b, 1, 0) == 0 &&
          clp_send(&q, 0, 2, &b, 1, 0) == 0);
    struct timespec deadline;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &deadline) == 0);
    deadline.tv_sec--;
    CHECK(clp_send(&q, 0, 3, &b, 1, &deadline) == ETIMEDOUT &&
          q.core.accepted == 2);
    struct cl_worker w = {&q, 0, 0, -1};
    pthread_t thread;
    CHECK(pthread_create(&thread, 0, one_sender, &w) == 0);
    enrolled(&q, 1);
    CHECK(pthread_mutex_lock(&q.mutex) == 0);
    CHECK(pthread_cond_broadcast(&q.writable) == 0);
    CHECK(pthread_mutex_unlock(&q.mutex) == 0);
    size_t n;
    uint32_t p, s;
    uint64_t ticket;
    CHECK(clp_recv(&q, &b, 1, &n, &p, &s, &ticket, 0) == 0 && s == 1);
    CHECK(pthread_join(thread, 0) == 0 && w.result == 0 &&
          q.core.accepted == 3);
    CHECK(clp_recv(&q, &b, 1, &n, &p, &s, &ticket, 0) == 0 && s == 2);
    CHECK(clp_recv(&q, &b, 1, &n, &p, &s, &ticket, 0) == 0 && s == 3 && b == 3);
    CHECK(clp_send(&q, 0, 4, &b, 1, 0) == 0 &&
          clp_send(&q, 0, 5, &b, 1, 0) == 0);
    w.result = -1;
    CHECK(pthread_create(&thread, 0, one_sender, &w) == 0);
    enrolled(&q, 1);
    CHECK(pthread_mutex_lock(&q.mutex) == 0);
    CHECK(pthread_cancel(thread) == 0);
    CHECK(q.active == 1 &&
          q.send_waiters ==
              1); /* cleanup cannot finish while this mutex is held */
    CHECK(pthread_mutex_unlock(&q.mutex) == 0);
    void *result = 0;
    CHECK(pthread_join(thread, &result) == 0 && result == PTHREAD_CANCELED);
    CHECK(q.active == 0 && q.send_waiters == 0 && q.core.accepted == 5);
    CHECK(pthread_create(&thread, 0, one_sender, &w) == 0);
    enrolled(&q, 1);
    CHECK(clp_close(&q, 1) == 0);
    CHECK(pthread_join(thread, 0) == 0 && w.result == EPIPE &&
          q.core.discarded == 2 && q.core.delivered == 3);
    CHECK(clp_destroy(&q) == 0 && clp_init(&q, 0) == 0);
    CHECK(pthread_create(&thread, 0, one_receiver, &w) == 0);
    enrolled(&q, 0);
    CHECK(clp_close(&q, 0) == 0);
    CHECK(pthread_join(thread, 0) == 0 && w.result == EPIPE &&
          clp_destroy(&q) == 0);
    struct cl_life life;
    struct cl_lease lease;
    cl_life_init(&life);
    CHECK(cl_acquire(&life, &lease) == CL_OK);
    CHECK(cl_transition(&life, lease, CL_ACTIVE, CL_TIMER) == CL_OK);
    struct cl_timer t = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0,
                         0};
    CHECK(pthread_create(&thread, 0, timer, &t) == 0);
    CHECK(pthread_mutex_lock(&t.lock) == 0);
    while (!t.entered)
        CHECK(pthread_cond_wait(&t.condition, &t.lock) == 0);
    cl_life_close(&life);
    CHECK(cl_life_drained(&life) == CL_OK && cl_destroy(&life) == CL_BUSY);
    t.stop = 1;
    CHECK(pthread_cond_signal(&t.condition) == 0);
    CHECK(cl_drop(&life, lease) ==
          CL_BUSY); /* requesting stop alone cannot release its lease */
    CHECK(pthread_mutex_unlock(&t.lock) == 0 && pthread_join(thread, 0) == 0);
    CHECK(cl_transition(&life, lease, CL_TIMER, CL_ACTIVE) == CL_OK &&
          cl_drop(&life, lease) == CL_OK && cl_destroy(&life) == CL_OK);
    CHECK(pthread_cond_destroy(&t.condition) == 0 &&
          pthread_mutex_destroy(&t.lock) == 0);
    puts("CL_OBSERVATION timeout=unaccepted cancelled=joined close=drained "
         "abort=2 timer=joined");
}

struct cl_test_owner
{
    pthread_mutex_t *mutex;
    struct clp_record *record;
    unsigned repairer;
    int result;
};

static void *dead_owner(void *arg)
{
    struct cl_test_owner *o = arg;
    int r = pthread_mutex_lock(o->mutex);
    CHECK(r == (o->repairer ? EOWNERDEAD : 0));
    o->record->value = o->repairer ? 19 : 13;
    return 0; /* intentionally exit owning the robust mutex */
}

static void *supervisor(void *arg)
{
    struct cl_test_owner *o = arg;
    o->result = clp_record_recover(o->mutex, o->record, 1);
    return 0;
}

static void robust(void)
{
    pthread_mutexattr_t attr;
    CHECK(pthread_mutexattr_init(&attr) == 0 &&
          pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) == 0);
    for (unsigned mode = 0; mode < 3; mode++)
    {
        pthread_mutex_t mutex;
        struct clp_record r = {7, ~UINT32_C(7), 7};
        CHECK(pthread_mutex_init(&mutex, &attr) == 0);
        struct cl_test_owner o = {&mutex, &r, 0, -1};
        pthread_t thread;
        CHECK(pthread_create(&thread, 0, dead_owner, &o) == 0 &&
              pthread_join(thread, 0) == 0);
        CHECK(r.value == 13 && r.checksum == ~UINT32_C(7));
        if (mode == 1)
        {
            o.repairer = 1;
            CHECK(pthread_create(&thread, 0, dead_owner, &o) == 0 &&
                  pthread_join(thread, 0) == 0 && r.value == 19);
        }
        if (mode == 2)
        {
            CHECK(clp_record_recover(&mutex, &r, 0) == ENOTRECOVERABLE);
            CHECK(pthread_mutex_lock(&mutex) == ENOTRECOVERABLE);
        }
        else
        {
            struct cl_test_owner second = {&mutex, &r, 0, -1};
            pthread_t other;
            CHECK(pthread_create(&thread, 0, supervisor, &o) == 0 &&
                  pthread_create(&other, 0, supervisor, &second) == 0);
            CHECK(pthread_join(thread, 0) == 0 && pthread_join(other, 0) == 0 &&
                  o.result == 0 && second.result == 0);
            CHECK(r.value == 7 && r.checksum == ~UINT32_C(7) &&
                  pthread_mutex_lock(&mutex) == 0 &&
                  pthread_mutex_unlock(&mutex) == 0);
        }
        CHECK(pthread_mutex_destroy(&mutex) == 0);
    }
    CHECK(pthread_mutexattr_destroy(&attr) == 0);
    puts(
        "CL_OBSERVATION robust=EOWNERDEAD repaired=7 repairer_death=EOWNERDEAD "
        "abandoned=ENOTRECOVERABLE");
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "threaded"))
        threaded();
    else if (!strcmp(argv[1], "waiting"))
        waiting();
    else if (!strcmp(argv[1], "robust"))
        robust();
    else
        return 2;
    printf("CL_PASS %s\n", argv[1]);
    return 0;
}
