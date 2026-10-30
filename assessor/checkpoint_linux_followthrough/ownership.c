/*
 * The same workload (sequence 0..63, payload [i, i ^ 0xa5], processed exactly
 * once, sum 2016) through two ownership designs:
 *
 *   copied   ingress copies each complete message into a queue; two workers
 *            each receive their own local value (the only shared mutable
 *            state is the result tally under a mutex and the cpa_queue,
 *            which has its own mutex);
 *   shared   one worker reads a single mutable cell guarded by a mutex and a
 *            condition variable, waiting on a predicate, never a notification.
 *
 * The two designs have different capacities and concurrency, so the program
 * claims no speed equivalence. Sequence 17 reports a worker error without
 * reopening ownership.
 */
#define _POSIX_C_SOURCE 200809L
#include "checkpoint_async.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                          \
    do                                                                    \
    {                                                                     \
        if (!(x))                                                         \
        {                                                                 \
            fprintf(stderr, "P5_OWNER_CHECK line=%d %s\n", __LINE__, #x); \
            exit(1);                                                      \
        }                                                                 \
    } while (0)

enum ownership_limit
{
    ITEMS = 64,
    COPY_QUEUE_CAPACITY = 128,
    COPY_WORKERS = 2,
    FAILING_SEQUENCE = 17,
    PAYLOAD_MASK = 0xa5,
    EXPECTED_SUM = 2016
};

/* Payload byte 1 of sequence i is i ^ PAYLOAD_MASK (byte 0 is i itself). */
static unsigned char check_byte(unsigned sequence)
{
    return (unsigned char)(sequence ^ PAYLOAD_MASK);
}

static struct cp_message make_message(unsigned sequence)
{
    struct cp_message m = {
        .sequence = (uint16_t)sequence,
        .length = 2,
        .payload = {(unsigned char)sequence, check_byte(sequence)}};
    return m;
}

struct p5_copies
{
    struct cpa_queue q;
    pthread_mutex_t results; /* Protects seen[] and sum only. */
    unsigned seen[ITEMS];
    unsigned sum;
};

/* Worker for the copied design: owns a local copy of every message it takes. */
static void *copied(void *arg)
{
    struct p5_copies *c = arg;
    for (;;)
    {
        struct cpa_handle h;
        struct cp_message m;
        const enum cpa_status s = cpa_take(&c->q, &h, &m, 1);
        if (s == CPA_CLOSED)
        {
            break;
        }
        CHECK(s == CPA_OK && m.sequence < ITEMS && m.length == 2 &&
              m.payload[0] == m.sequence &&
              m.payload[1] == check_byte(m.sequence));
        /* Decoder/input is not shared. Worker owns a local mutable copy only. */
        CHECK(!pthread_mutex_lock(&c->results));
        c->sum += m.payload[0];
        ++c->seen[m.sequence];
        CHECK(!pthread_mutex_unlock(&c->results));
        CHECK(cpa_finish(&c->q, h, m.sequence == FAILING_SEQUENCE) == CPA_OK);
    }
    return 0;
}

struct p5_shared
{
    pthread_mutex_t mutex; /* Protects every field below. */
    pthread_cond_t changed;
    struct cp_message message;
    unsigned seen[ITEMS];
    unsigned sum;
    int ready;
    int closed;
};

/* Worker for the shared design: consumes the single cell under the mutex. */
static void *mutable(void *arg)
{
    struct p5_shared *s = arg;
    CHECK(!pthread_mutex_lock(&s->mutex));
    for (;;)
    {
        while (!s->ready && !s->closed)
        {
            CHECK(!pthread_cond_wait(&s->changed, &s->mutex));
        }
        if (!s->ready)
        {
            break;
        }
        CHECK(s->message.sequence < ITEMS &&
              s->message.payload[0] == s->message.sequence &&
              s->message.payload[1] == check_byte(s->message.sequence));
        s->sum += s->message.payload[0];
        ++s->seen[s->message.sequence];
        memset(s->message.payload, 0, sizeof s->message.payload);
        s->ready = 0;
        CHECK(!pthread_cond_broadcast(&s->changed));
    }
    CHECK(!pthread_mutex_unlock(&s->mutex));
    return 0;
}

static void check_each_sequence_once(const unsigned *seen)
{
    for (unsigned i = 0; i < ITEMS; i++)
    {
        CHECK(seen[i] == 1);
    }
}

static void run_copied_design(void)
{
    struct p5_copies c = {0};
    CHECK(cpa_init(&c.q, COPY_QUEUE_CAPACITY) == CPA_OK &&
          !pthread_mutex_init(&c.results, 0));
    pthread_t threads[COPY_WORKERS];
    for (unsigned i = 0; i < COPY_WORKERS; i++)
    {
        CHECK(!pthread_create(&threads[i], 0, copied, &c));
    }
    for (unsigned i = 0; i < ITEMS; i++)
    {
        struct cp_message m = make_message(i);
        CHECK(cpa_offer(&c.q, &m, 1) == CPA_OK);
        memset(&m, 0, sizeof m); /* The queue holds its own copy. */
    }
    const enum cpa_status status = cpa_close(&c.q, 0);
    CHECK(status == CPA_PENDING || status == CPA_CLOSED);
    for (unsigned i = 0; i < COPY_WORKERS; i++)
    {
        CHECK(!pthread_join(threads[i], 0));
    }
    CHECK(c.sum == EXPECTED_SUM && c.q.accepted == ITEMS &&
          c.q.completed == ITEMS && c.q.releases == ITEMS &&
          c.q.report_failures == 1);
    check_each_sequence_once(c.seen);
    CHECK(cpa_destroy(&c.q) == CPA_OK && !pthread_mutex_destroy(&c.results));
}

static void run_shared_design(void)
{
    struct p5_shared s = {0};
    pthread_t worker;
    CHECK(!pthread_mutex_init(&s.mutex, 0) &&
          !pthread_cond_init(&s.changed, 0));
    CHECK(!pthread_create(&worker, 0, mutable, &s));
    for (unsigned i = 0; i < ITEMS; i++)
    {
        CHECK(!pthread_mutex_lock(&s.mutex));
        while (s.ready)
        {
            CHECK(!pthread_cond_wait(&s.changed, &s.mutex));
        }
        s.message = make_message(i);
        s.ready = 1;
        CHECK(!pthread_cond_broadcast(&s.changed));
        CHECK(!pthread_mutex_unlock(&s.mutex));
    }
    CHECK(!pthread_mutex_lock(&s.mutex));
    s.closed = 1;
    CHECK(!pthread_cond_broadcast(&s.changed));
    CHECK(!pthread_mutex_unlock(&s.mutex));
    CHECK(!pthread_join(worker, 0));
    CHECK(s.sum == EXPECTED_SUM);
    check_each_sequence_once(s.seen);
    CHECK(!pthread_cond_destroy(&s.changed) &&
          !pthread_mutex_destroy(&s.mutex));
}

int main(void)
{
    run_copied_design();
    run_shared_design();
    puts(
        "P5_OWNER_PASS same_workload=64 sum=2016 copy_workers=2 mutable_workers=1 no_performance_equivalence_claim");
    return 0;
}
