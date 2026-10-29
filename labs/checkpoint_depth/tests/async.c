#define _POSIX_C_SOURCE 200809L
#include "checkpoint_async.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "CP_CHECK: %s:%d %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

static struct cp_message message(unsigned seq)
{
    struct cp_message m = {0};
    m.sequence = (uint16_t)seq;
    m.length = 2;
    m.payload[0] = (unsigned char)seq;
    m.payload[1] = 0xa5;
    return m;
}

static struct timespec deadline(void)
{
    struct timespec t;
    CHECK(!clock_gettime(CLOCK_MONOTONIC, &t));
    return t;
}

static void consume(struct cpa_queue *q, unsigned expected)
{
    struct cpa_handle h;
    struct cp_message m;
    CHECK(cpa_take(q, &h, &m, 0) == CPA_OK);
    CHECK(m.sequence == expected && m.payload[0] == expected &&
          m.payload[1] == 0xa5);
    CHECK(cpa_finish(q, h, 0) == CPA_OK);
    CHECK(cpa_finish(q, h, 0) == CPA_STALE);
}

void cp_test_async_admission(void)
{
    struct cpa_queue q;
    CHECK(cpa_init(&q, 32) == CPA_OK);
    struct cp_message m = message(1);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    m = message(2);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    consume(&q, 1);
    m = message(3);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    m = message(4);
    CHECK(cpa_offer(&q, &m, 0) == CPA_FULL);
    consume(&q, 2);
    m = message(5);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    m = message(6);
    CHECK(cpa_offer(&q, &m, 0) == CPA_FULL);
    consume(&q, 3);
    consume(&q, 5);
    CHECK(q.accepted == 4 && q.completed == 4 && q.rejected == 2 &&
          q.releases == 4);
    /* Old completion cannot release a newly reused live slot. */
    m = message(7);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    struct cpa_handle old, current;
    struct cp_message got;
    CHECK(cpa_take(&q, &old, &got, 0) == CPA_OK &&
          cpa_finish(&q, old, 0) == CPA_OK);
    m = message(8);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK &&
          cpa_take(&q, &current, &got, 0) == CPA_OK);
    CHECK(old.slot == current.slot && old.generation != current.generation);
    CHECK(cpa_finish(&q, old, 0) == CPA_STALE && q.owned == 1);
    CHECK(cpa_finish(&q, current, 1) == CPA_OK && q.report_failures == 1);
    CHECK(cpa_close(&q, 0) == CPA_CLOSED && cpa_destroy(&q) == CPA_OK);
    CHECK(cpa_init(&q, 1) == CPA_OK);
    for (unsigned i = 0; i < CPA_SLOTS; ++i)
    {
        m = message(i + 1);
        CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
        consume(&q, i + 1);
    }
    CHECK(cpa_offer(&q, &m, 1) == CPA_FULL);
    CHECK(cpa_close(&q, 0) == CPA_CLOSED && cpa_destroy(&q) == CPA_OK);
}

void cp_test_async_lifecycle(void)
{
    struct cpa_queue q;
    CHECK(cpa_init(&q, 3) == CPA_OK);
    struct cp_message m = message(1), owned;
    struct cpa_handle h;
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK &&
          cpa_take(&q, &h, &owned, 0) == CPA_OK);
    m = message(2);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    m = message(3);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    CHECK(cpa_close(&q, 1) == CPA_PENDING && q.owned == 1 && q.discarded == 2 &&
          q.releases == 2);
    struct timespec until = deadline();
    CHECK(cpa_wait_closed(&q, &until) == CPA_PENDING && q.owned == 1 &&
          q.state == CPA_ABORTING);
    CHECK(cpa_destroy(&q) == CPA_PENDING);
    CHECK(owned.payload[0] == 1 && cpa_finish(&q, h, 0) == CPA_OK &&
          q.state == CPA_STOPPED);
    CHECK(cpa_finish(&q, h, 0) == CPA_STALE && q.releases == 3 &&
          q.completed == 1);
    CHECK(cpa_close(&q, 1) == CPA_CLOSED && q.releases == 3 &&
          cpa_destroy(&q) == CPA_OK);
    CHECK(cpa_init(&q, 3) == CPA_OK);
    m = message(9);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    CHECK(cpa_close(&q, 0) == CPA_PENDING);
    consume(&q, 9);
    CHECK(q.transitions == (1u << CPA_OPEN | 1u << CPA_CLOSING |
                            1u << CPA_DRAINING | 1u << CPA_STOPPED));
    CHECK(cpa_destroy(&q) == CPA_OK);
}

struct cpx_thread
{
    struct cpa_queue *queue;
    enum cpa_status status;
    struct cpa_handle handle;
    struct cp_message message;
    int producer;
};

static void *run_thread(void *value)
{
    struct cpx_thread *t = value;
    t->status = t->producer ? cpa_offer(t->queue, &t->message, 1)
                            : cpa_take(t->queue, &t->handle, &t->message, 1);
    return 0;
}

static void await_metric(struct cpa_queue *q, int wakes, size_t minimum)
{
    for (unsigned i = 0; i < 5000; ++i)
    {
        CHECK(!pthread_mutex_lock(&q->mutex));
        size_t value = wakes ? q->wakes : q->waiters;
        CHECK(!pthread_mutex_unlock(&q->mutex));
        if (value >= minimum)
            return;
        struct timespec pause = {0, 1000000};
        (void)nanosleep(&pause, 0);
    }
    CHECK(0);
}

void cp_test_async_wait(void)
{
    struct cpa_queue q;
    CHECK(cpa_init(&q, 32) == CPA_OK);
    struct cp_message m = message(1);
    /* Signal before waiter exists: the predicate, not notification history, supplies data. */
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    struct cpx_thread first = {.queue = &q};
    pthread_t a;
    CHECK(!pthread_create(&a, 0, run_thread, &first));
    CHECK(!pthread_join(a, 0));
    CHECK(first.status == CPA_OK);
    CHECK(cpa_finish(&q, first.handle, 0) == CPA_OK);
    struct cpx_thread second = {.queue = &q};
    CHECK(!pthread_create(&a, 0, run_thread, &second));
    await_metric(&q, 0, 1);
    CHECK(!pthread_mutex_lock(&q.mutex));
    size_t before = q.wakes;
    CHECK(!pthread_cond_broadcast(&q.changed));
    CHECK(!pthread_mutex_unlock(&q.mutex));
    await_metric(&q, 1, before + 1);
    m = message(2);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    CHECK(!pthread_join(a, 0));
    CHECK(second.status == CPA_OK && second.message.sequence == 2);
    CHECK(cpa_finish(&q, second.handle, 0) == CPA_OK);
    /* Two competing consumers; one item is owned by exactly one, closure releases the other. */
    struct cpx_thread pair[2] = {{.queue = &q}, {.queue = &q}};
    pthread_t ids[2];
    for (unsigned i = 0; i < 2; ++i)
        CHECK(!pthread_create(&ids[i], 0, run_thread, &pair[i]));
    await_metric(&q, 0, 2);
    m = message(3);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    CHECK(cpa_close(&q, 0) == CPA_PENDING);
    unsigned owners = 0;
    for (unsigned i = 0; i < 2; ++i)
    {
        CHECK(!pthread_join(ids[i], 0));
        if (pair[i].status == CPA_OK)
        {
            ++owners;
            CHECK(pair[i].message.sequence == 3);
            CHECK(cpa_finish(&q, pair[i].handle, 0) == CPA_OK);
        }
        else
            CHECK(pair[i].status == CPA_CLOSED);
    }
    CHECK(owners == 1 && cpa_destroy(&q) == CPA_OK);
    CHECK(cpa_init(&q, 32) == CPA_OK);
    m = message(4);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    CHECK(cpa_offer(&q, &m, 0) == CPA_OK);
    struct cpx_thread producer = {.queue = &q, .message = m, .producer = 1};
    CHECK(!pthread_create(&a, 0, run_thread, &producer));
    await_metric(&q, 0, 1);
    CHECK(cpa_close(&q, 1) == CPA_CLOSED);
    CHECK(!pthread_join(a, 0));
    CHECK(producer.status == CPA_CLOSED && q.discarded == 2 &&
          cpa_destroy(&q) == CPA_OK);
}
