#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "registration.h"
#include "cost_reference.h"
#include "weak.h"
#include "stream.h"
#include "nursery.h"
#include "supervisor.h"
#include "transactions.h"
#include "versions.h"
#include "operation.h"
#include "actor_executor.h"
#include "completion.h"
#include <inttypes.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "SC_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

enum
{
    SELECT_SHARED,
    SELECT_REGISTERED,
    DEQUE_SC,
    DEQUE_WEAK,
    DEQUE_MUTEX,
    SEQUENCE_SC,
    SEQUENCE_WEAK,
    SEQUENCE_MUTEX,
    TRANSACTION,
    MUTEX_TRANSACTION
};

struct msg_sample
{
    uint64_t *latency, retries, success, checksum;
    size_t count;
};
struct msg_bench;

struct msg_arg
{
    struct msg_bench *bench;
    unsigned id;
};

struct msg_bench
{
    int mode;
    unsigned n;
    struct msg_sample sample[5];
    atomic_bool start, done;
    atomic_uint ready;
    atomic_uint *seen;
    struct msg_hub shared;
    struct mfr_hub registered;
    struct msg_deque scdeque;
    struct mfw_deque weakdeque;
    struct mfm_deque mutexdeque;
    struct msg_sequence scsequence;
    struct mfw_sequence weaksequence;
    struct mfm_pair pair;
    struct msg_store store;
};

static uint64_t now(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static void record(struct msg_sample *s, uint64_t begin)
{
    s->latency[s->count++] = now() - begin;
}

static void output_threads(const char *name, struct msg_sample *s,
                           unsigned count, uint64_t elapsed, uint64_t extra)
{
    printf("{\"mode\":\"%s\",\"elapsed_ns\":%" PRIu64 ",\"extra\":%" PRIu64
           ",\"threads\":[",
           name, elapsed, extra);
    for (unsigned k = 0; k < count; k++)
    {
        if (k)
            putchar(',');
        printf("{\"success\":%" PRIu64 ",\"retries\":%" PRIu64
               ",\"checksum\":%" PRIu64 ",\"latency_ns\":[",
               s[k].success, s[k].retries, s[k].checksum);
        for (size_t j = 0; j < s[k].count; j++)
        {
            if (j)
                putchar(',');
            printf("%" PRIu64, s[k].latency[j]);
        }
        printf("]}");
    }
    printf("]}\n");
}

static void output(const char *name, struct msg_sample s[3], uint64_t elapsed,
                   uint64_t extra)
{
    output_threads(name, s, 3, elapsed, extra);
}

static void *work(void *opaque)
{
    struct msg_arg *arg = opaque;
    struct msg_bench *b = arg->bench;
    unsigned id = arg->id, n = b->n;
    struct msg_sample *s = &b->sample[id];
    atomic_fetch_add_explicit(&b->ready, 1, memory_order_release);
    while (!atomic_load_explicit(&b->start, memory_order_acquire))
        sched_yield();
    if (b->mode <= SELECT_REGISTERED)
    {
        if (id < 2)
        {
            for (unsigned i = 0; i < n; i++)
            {
                uint64_t begin = now();
                enum msg_status status;
                do
                {
                    status = b->mode == SELECT_SHARED
                                 ? msg_send(&b->shared, id, id * n + i, false)
                                 : mfr_send(&b->registered, id, id * n + i);
                    if (status == MSG_FULL)
                    {
                        ++s->retries;
                        sched_yield();
                    }
                    CHECK(s->retries < UINT64_C(10000000));
                } while (status == MSG_FULL);
                CHECK(status == MSG_OK);
                ++s->success;
                record(s, begin);
            }
            CHECK((b->mode == SELECT_SHARED
                       ? msg_close(&b->shared, id)
                       : mfr_close(&b->registered, id)) == MSG_OK);
        }
        else
            for (;;)
            {
                unsigned which = 9, value = 9;
                uint64_t begin = now();
                enum msg_status status =
                    b->mode == SELECT_SHARED
                        ? msg_select(&b->shared, 3, true, &which, &value)
                        : mfr_select(&b->registered, 3, true, &which, &value);
                if (status == MSG_CLOSED)
                    break;
                CHECK(status == MSG_OK && value < 2 * n && which == value / n);
                CHECK(atomic_fetch_add(&b->seen[value], 1) == 0);
                ++s->success;
                s->checksum += value;
                record(s, begin);
            }
    }
    else if (b->mode <= DEQUE_MUTEX)
    {
        if (!id)
        {
            for (unsigned i = 0; i < n; i++)
            {
                uint64_t begin = now();
                int status;
                do
                {
                    status = b->mode == DEQUE_SC ? (int)msg_push(&b->scdeque, i)
                             : b->mode == DEQUE_WEAK
                                 ? (int)mfw_push(&b->weakdeque, i)
                                 : (int)mfm_push(&b->mutexdeque, i);
                    bool full = b->mode == DEQUE_WEAK ? status == MFW_FULL
                                                      : status == MSG_FULL;
                    if (!full)
                        break;
                    ++s->retries;
                    CHECK(s->retries < UINT64_C(10000000));
                    sched_yield();
                } while (true);
                CHECK(status == 0);
                ++s->success;
                record(s, begin);
            }
            atomic_store_explicit(&b->done, true, memory_order_release);
        }
        else
            for (;;)
            {
                unsigned value = 0;
                uint64_t begin = now();
                int status = b->mode == DEQUE_SC
                                 ? (int)msg_steal(&b->scdeque, &value)
                             : b->mode == DEQUE_WEAK
                                 ? (int)mfw_steal(&b->weakdeque, &value)
                                 : (int)mfm_steal(&b->mutexdeque, &value);
                if (!status)
                {
                    CHECK(value < n &&
                          atomic_fetch_add(&b->seen[value], 1) == 0);
                    ++s->success;
                    s->checksum += value;
                    record(s, begin);
                }
                else
                {
                    bool empty = b->mode == DEQUE_WEAK ? status == MFW_EMPTY
                                                       : status == MSG_EMPTY;
                    bool conflict = b->mode == DEQUE_WEAK
                                        ? status == MFW_CONFLICT
                                        : status == MSG_CONFLICT;
                    CHECK(empty || conflict);
                    ++s->retries;
                    CHECK(s->retries < UINT64_C(10000000));
                    if (empty &&
                        atomic_load_explicit(&b->done, memory_order_acquire))
                    {
                        /* Recheck after observing the final producer publication. */
                        status = b->mode == DEQUE_SC
                                     ? (int)msg_steal(&b->scdeque, &value)
                                 : b->mode == DEQUE_WEAK
                                     ? (int)mfw_steal(&b->weakdeque, &value)
                                     : (int)mfm_steal(&b->mutexdeque, &value);
                        if (status == (b->mode == DEQUE_WEAK ? (int)MFW_EMPTY
                                                             : (int)MSG_EMPTY))
                            break;
                        if (!status)
                        {
                            CHECK(value < n &&
                                  atomic_fetch_add(&b->seen[value], 1) == 0);
                            ++s->success;
                            s->checksum += value;
                            record(s, begin);
                        }
                    }
                    sched_yield();
                }
            }
    }
    else if (b->mode <= SEQUENCE_MUTEX)
    {
        for (unsigned i = 1; i <= n; i++)
        {
            uint64_t begin = now();
            if (!id)
            {
                if (b->mode == SEQUENCE_SC)
                    CHECK(msg_sequence_write(&b->scsequence, i, ~i) == MSG_OK);
                else if (b->mode == SEQUENCE_WEAK)
                    CHECK(mfw_write(&b->weaksequence, i, ~i) == MFW_OK);
                else
                    mfm_pair_write(&b->pair, i, ~i);
                ++s->success;
            }
            else
            {
                unsigned values[2] = {999, 999};
                bool ok;
                if (b->mode == SEQUENCE_SC)
                    ok = msg_sequence_read(&b->scsequence, 1, values) == MSG_OK;
                else if (b->mode == SEQUENCE_WEAK)
                    ok = mfw_read(&b->weaksequence, 1, values) == MFW_OK;
                else
                    ok = mfm_pair_try_read(&b->pair, values);
                if (ok)
                {
                    CHECK((!values[0] && !values[1]) ||
                          values[1] == ~values[0]);
                    ++s->success;
                    s->checksum += values[0];
                }
                else
                {
                    CHECK(values[0] == 999 && values[1] == 999);
                    ++s->retries;
                }
            }
            record(s, begin);
        }
    }
    else
        for (unsigned i = 0; i < n; i++)
        {
            uint64_t begin = now();
            if (b->mode == MUTEX_TRANSACTION)
                mfm_pair_increment(&b->pair);
            else
                for (;;)
                {
                    struct msg_txn tx;
                    unsigned x, y;
                    msg_begin(&b->store, &tx);
                    enum msg_status status = msg_read(&tx, 0, &x);
                    if (status == MSG_OK)
                        status = msg_read(&tx, 1, &y);
                    if (status == MSG_OK)
                    {
                        CHECK(x == y);
                        CHECK(msg_write(&tx, 0, x + 1) == MSG_OK &&
                              msg_write(&tx, 1, y + 1) == MSG_OK);
                        status = msg_commit(&tx);
                    }
                    else
                        msg_abort(&tx);
                    if (status == MSG_OK)
                        break;
                    CHECK(status == MSG_CONFLICT);
                    ++s->retries;
                    CHECK(s->retries < UINT64_C(10000000));
                    sched_yield();
                }
            ++s->success;
            record(s, begin);
        }
    return NULL;
}

static void threaded(const char *name, int mode, unsigned n, unsigned count)
{
    struct msg_bench b = {.mode = mode, .n = n};
    atomic_init(&b.start, false);
    atomic_init(&b.done, false);
    atomic_init(&b.ready, 0);
    b.seen = calloc(2 * n, sizeof *b.seen);
    CHECK(b.seen);
    for (unsigned i = 0; i < 2 * n; i++)
        atomic_init(&b.seen[i], 0);
    for (unsigned k = 0; k < count; k++)
    {
        b.sample[k].latency = calloc(2 * n, sizeof(uint64_t));
        CHECK(b.sample[k].latency);
    }
    if (mode == SELECT_SHARED)
        CHECK(msg_hub_init(&b.shared, 4, 4) == MSG_OK);
    if (mode == SELECT_REGISTERED)
        CHECK(mfr_init(&b.registered, 4, 4) == MSG_OK);
    if (mode == DEQUE_SC)
        msg_deque_init(&b.scdeque);
    if (mode == DEQUE_WEAK)
        mfw_deque_init(&b.weakdeque);
    if (mode == DEQUE_MUTEX)
        mfm_deque_init(&b.mutexdeque);
    if (mode == SEQUENCE_SC)
        msg_sequence_init(&b.scsequence);
    if (mode == SEQUENCE_WEAK)
        mfw_sequence_init(&b.weaksequence);
    if (mode == SEQUENCE_MUTEX || mode == MUTEX_TRANSACTION)
        mfm_pair_init(&b.pair);
    if (mode == TRANSACTION)
        msg_store_init(&b.store);
    CHECK(count <= 5);
    pthread_t threads[5];
    struct msg_arg args[5];
    for (unsigned i = 0; i < count; i++)
    {
        args[i] = (struct msg_arg){&b, i};
        CHECK(pthread_create(&threads[i], NULL, work, &args[i]) == 0);
    }
    while (atomic_load_explicit(&b.ready, memory_order_acquire) != count)
        sched_yield();
    uint64_t begin = now();
    atomic_store_explicit(&b.start, true, memory_order_release);
    for (unsigned i = 0; i < count; i++)
        CHECK(pthread_join(threads[i], NULL) == 0);
    uint64_t elapsed = now() - begin, extra = 0;
    if (mode <= DEQUE_MUTEX)
    {
        unsigned total = mode <= SELECT_REGISTERED ? 2 * n : n;
        for (unsigned i = 0; i < total; i++)
            CHECK(atomic_load(&b.seen[i]) == 1);
    }
    if (mode == SELECT_SHARED)
        msg_hub_destroy(&b.shared);
    if (mode == SELECT_REGISTERED)
    {
        size_t waiting;
        mfr_observe(&b.registered, &waiting, &extra);
        CHECK(!waiting);
        CHECK(mfr_destroy(&b.registered) == MSG_OK);
    }
    if (mode == DEQUE_MUTEX)
        mfm_deque_destroy(&b.mutexdeque);
    if (mode == TRANSACTION)
    {
        CHECK(atomic_load(&b.store.cells[0].value) == 2 * n &&
              atomic_load(&b.store.cells[1].value) == 2 * n);
        extra = atomic_load(&b.store.clock);
    }
    if (mode == MUTEX_TRANSACTION)
    {
        CHECK(b.pair.values[0] == 2 * n && b.pair.values[1] == 2 * n);
    }
    if (mode == SEQUENCE_MUTEX || mode == MUTEX_TRANSACTION)
        mfm_pair_destroy(&b.pair);
    output_threads(name, b.sample, count > 3 ? count : 3, elapsed, extra);
    for (unsigned k = 0; k < count; k++)
        free(b.sample[k].latency);
    free(b.seen);
}

static void snapshots(bool pins, unsigned n)
{
    struct msg_sample s[3] = {{0}};
    s[0].latency = calloc(n, sizeof(uint64_t));
    CHECK(s[0].latency);
    struct msg_versions versions;
    struct mfm_pair pair;
    if (pins)
        CHECK(msg_versions_init(&versions) == MSG_OK);
    else
        mfm_pair_init(&pair);
    uint64_t begin = now(), peak = 0;
    for (unsigned i = 1; i <= n; i++)
    {
        uint64_t start = now();
        unsigned copy[2];
        struct msg_snapshot pin = {0};
        if (pins)
        {
            CHECK(msg_publish(&versions, i, ~i) == MSG_OK);
            CHECK(msg_snapshot(&versions, &pin) == MSG_OK);
            CHECK(msg_publish(&versions, i + 1, ~(i + 1)) == MSG_OK);
        }
        else
        {
            mfm_pair_write(&pair, i, ~i);
            mfm_pair_copy(&pair, copy);
            mfm_pair_write(&pair, i + 1, ~(i + 1));
        }
        for (unsigned j = 0; j < 8; j++)
        {
            unsigned out[2];
            if (pins)
                CHECK(msg_snapshot_read(&versions, pin, out) == MSG_OK);
            else
            {
                out[0] = copy[0];
                out[1] = copy[1];
            }
            CHECK(out[0] == i && out[1] == ~i);
            s[0].checksum += out[0];
        }
        if (pins)
        {
            unsigned live = 0;
            for (unsigned k = 0; k < 8; k++)
                if (versions.versions[k].live)
                    ++live;
            if (live > peak)
                peak = live;
            CHECK(msg_snapshot_drop(&versions, pin) == MSG_OK);
        }
        ++s[0].success;
        record(&s[0], start);
    }
    uint64_t elapsed = now() - begin;
    if (pins)
        msg_versions_destroy(&versions);
    else
        mfm_pair_destroy(&pair);
    output(pins ? "snapshot_pins" : "snapshot_copy", s, elapsed, peak);
    free(s[0].latency);
}

static void snapshots_four(bool pins, unsigned n)
{
    struct msg_sample s[3] = {{0}};
    s[0].latency = calloc(n, sizeof(uint64_t));
    CHECK(s[0].latency);
    struct msg_versions versions;
    struct mfm_pair pair;
    if (pins)
        CHECK(msg_versions_init(&versions) == MSG_OK);
    else
        mfm_pair_init(&pair);
    uint64_t begin = now(), peak = 0;
    for (unsigned i = 1; i <= n; i++)
    {
        uint64_t start = now();
        unsigned copies[4][2];
        struct msg_snapshot handles[4];
        for (unsigned j = 0; j < 4; j++)
        {
            unsigned value = 4 * i + j;
            if (pins)
            {
                CHECK(msg_publish(&versions, value, ~value) == MSG_OK);
                CHECK(msg_snapshot(&versions, &handles[j]) == MSG_OK);
            }
            else
            {
                mfm_pair_write(&pair, value, ~value);
                mfm_pair_copy(&pair, copies[j]);
            }
        }
        if (pins)
        {
            struct msg_snapshot extra = {7, UINT64_MAX};
            CHECK(msg_snapshot(&versions, &extra) == MSG_FULL &&
                  extra.slot == 7 && extra.generation == UINT64_MAX);
            CHECK(msg_publish(&versions, 4 * i + 4, ~(4 * i + 4)) == MSG_OK);
        }
        else
            mfm_pair_write(&pair, 4 * i + 4, ~(4 * i + 4));
        for (unsigned j = 0; j < 4; j++)
            for (unsigned k = 0; k < 8; k++)
            {
                unsigned out[2];
                if (pins)
                    CHECK(msg_snapshot_read(&versions, handles[j], out) ==
                          MSG_OK);
                else
                {
                    out[0] = copies[j][0];
                    out[1] = copies[j][1];
                }
                CHECK(out[0] == 4 * i + j && out[1] == ~(4 * i + j));
                s[0].checksum += out[0];
            }
        if (pins)
        {
            unsigned live = 0;
            for (unsigned j = 0; j < 8; j++)
                if (versions.versions[j].live)
                    ++live;
            if (live > peak)
                peak = live;
            /* Non-LIFO release must not reclaim another reader's selection. */
            const unsigned order[4] = {1, 3, 0, 2};
            for (unsigned j = 0; j < 4; j++)
                CHECK(msg_snapshot_drop(&versions, handles[order[j]]) ==
                      MSG_OK);
        }
        ++s[0].success;
        record(&s[0], start);
    }
    uint64_t elapsed = now() - begin;
    if (pins)
        msg_versions_destroy(&versions);
    else
        mfm_pair_destroy(&pair);
    output(pins ? "snapshot_pins4" : "snapshot_copy4", s, elapsed, peak);
    free(s[0].latency);
}

struct msg_sink
{
    pthread_mutex_t lock;
    pthread_cond_t changed;
    unsigned count, dropped;
    uint64_t checksum;
};

static void next(void *p, unsigned v)
{
    struct msg_sink *s = p;
    pthread_mutex_lock(&s->lock);
    ++s->count;
    s->checksum += v;
    pthread_cond_signal(&s->changed);
    pthread_mutex_unlock(&s->lock);
}

static void terminal(void *p, enum msg_status status)
{
    (void)p;
    CHECK(status == MSG_OK);
}

static void drop(void *p)
{
    struct msg_sink *s = p;
    ++s->dropped;
}

static void streams(unsigned batch, unsigned n)
{
    struct msg_sink sink = {0};
    CHECK(pthread_mutex_init(&sink.lock, NULL) == 0 &&
          pthread_cond_init(&sink.changed, NULL) == 0);
    struct mfq_stream stream;
    struct mfo_value owner = {&sink, drop};
    CHECK(mfq_init(&stream, 4, &owner, next, terminal, false) == MSG_OK);
    struct mfq_handle handle = {0};
    CHECK(mfq_acquire(&stream, &handle) == MSG_OK);
    struct msg_sample s[3] = {{0}};
    s[0].latency = calloc(n, sizeof(uint64_t));
    CHECK(s[0].latency);
    uint64_t calls = 0, begin = now();
    for (unsigned i = 0; i < n; i++)
    {
        uint64_t start = now();
        if (i % batch == 0)
        {
            unsigned amount = n - i < batch ? n - i : batch;
            CHECK(mfq_request(&handle, amount) == MSG_OK);
            ++calls;
        }
        CHECK(mfq_submit(&handle, i, MFQ_WAIT) == MSG_OK);
        ++s[0].success;
        record(&s[0], start);
    }
    pthread_mutex_lock(&sink.lock);
    while (sink.count < n)
        pthread_cond_wait(&sink.changed, &sink.lock);
    CHECK(sink.count == n && sink.checksum == (uint64_t)n * (n - 1) / 2);
    s[0].checksum = sink.checksum;
    pthread_mutex_unlock(&sink.lock);
    uint64_t elapsed = now() - begin;
    CHECK(mfq_close(&stream) == MSG_OK && mfq_join(&stream) == MSG_OK);
    mfq_release(&handle);
    CHECK(mfq_destroy(&stream) == MSG_OK && sink.dropped == 1);
    output(batch == 1 ? "stream_item" : "stream_batch16", s, elapsed, calls);
    free(s[0].latency);
    CHECK(pthread_cond_destroy(&sink.changed) == 0 &&
          pthread_mutex_destroy(&sink.lock) == 0);
}

static void representation(void)
{
    printf("{\"mode\":\"representation\",\"types\":{");
#define TYPE(t) \
    printf("\"" #t "\":{\"bytes\":%zu,\"align\":%zu},", sizeof(t), _Alignof(t))
    TYPE(struct msg_nursery);
    TYPE(struct msg_hub);
    TYPE(struct msg_channel);
    TYPE(struct msg_operation);
    TYPE(struct msg_actor);
    TYPE(struct msg_supervisor);
    TYPE(struct msg_deque);
    TYPE(struct msg_store);
    TYPE(struct msg_txn);
    TYPE(struct msg_versions);
    TYPE(struct msg_sequence);
    TYPE(struct msg_stream);
    TYPE(struct mfn_tree);
    TYPE(struct mfc_hub);
    TYPE(struct mfo_operation);
    TYPE(struct mfx_completion);
    TYPE(struct mfa_actor);
    TYPE(struct mfe_executor);
    TYPE(struct mfe_envelope);
    TYPE(struct mfs_tree);
    TYPE(struct mfg_deque);
    TYPE(struct mfp_pointer);
    TYPE(struct mfv_store);
    TYPE(struct mfk_store);
    TYPE(struct mfk_txn);
    TYPE(struct mft_store);
    TYPE(struct mft_txn);
    TYPE(struct mfq_stream);
    TYPE(struct mfr_hub);
    TYPE(struct mfm_pair);
    TYPE(struct mfm_deque);
    TYPE(struct mfw_deque);
    TYPE(struct mfw_sequence);
    TYPE(struct mfo_value);
    TYPE(pthread_mutex_t);
    TYPE(pthread_cond_t);
    printf("\"unsigned\":{\"bytes\":%zu,\"align\":%zu}},", sizeof(unsigned),
           _Alignof(unsigned));
    atomic_uint u;
    atomic_size_t z;
    atomic_init(&u, 0);
    atomic_init(&z, 0);
    printf("\"atomic_uint_lock_free\":%s,\"atomic_size_t_lock_free\":%s}\n",
           atomic_is_lock_free(&u) ? "true" : "false",
           atomic_is_lock_free(&z) ? "true" : "false");
}

static void fairness(unsigned n)
{
    struct mfw_deque q;
    mfw_deque_init(&q);
    unsigned a_conflicts = 0, b_wins = 0;
    for (unsigned i = 0; i < n; i++)
    {
        CHECK(mfw_push(&q, i) == MFW_OK);
        /* Replay A's real steal prefix, then B's whole steal, then A's CAS.
         * This is a controlled C atomic interleaving, not a scheduler sample. */
        size_t top = atomic_load_explicit(&q.top, memory_order_acquire);
        atomic_thread_fence(memory_order_seq_cst);
        size_t bottom = atomic_load_explicit(&q.bottom, memory_order_acquire);
        CHECK(top < bottom);
        unsigned a = atomic_load_explicit(&q.slots[top % 8],
                                          memory_order_relaxed),
                 b = 0;
        CHECK(a == i && mfw_steal(&q, &b) == MFW_OK && b == i);
        ++b_wins;
        CHECK(!atomic_compare_exchange_strong_explicit(
            &q.top, &top, top + 1, memory_order_seq_cst, memory_order_relaxed));
        ++a_conflicts;
    }
    printf(
        "{\"mode\":\"fairness\",\"rounds\":%u,\"a_attempts\":%u,\"a_success\":0,\"b_success\":%u}\n",
        n, a_conflicts, b_wins);
}

struct msg_counts
{
    uint64_t calls, drops, checksum, opens, closes;
};

struct msg_value
{
    unsigned value;
    struct msg_counts *counts;
};

static void value_drop(void *p)
{
    struct msg_value *v = p;
    ++v->counts->drops;
}

static bool value_handle(void *p, const struct mfo_value *owner)
{
    struct msg_counts *c = p;
    struct msg_value *v = owner->data;
    ++c->calls;
    c->checksum += v->value;
    return true;
}

static void value_dead(void *p, uint64_t generation, struct mfo_value owner)
{
    (void)p;
    (void)generation;
    mfo_drop(&owner);
}

static void value_receive(void *p, enum msg_status status,
                          struct mfo_value owner)
{
    CHECK(status == MSG_OK);
    CHECK(value_handle(p, &owner));
    mfo_drop(&owner);
}

static void value_backend(struct mfo_operation *op, void *p)
{
    CHECK(mfo_finish(op, MSG_OK, p) == MSG_OK);
}

static bool resource_open(void *p, unsigned group, unsigned child)
{
    (void)group;
    (void)child;
    ++((struct msg_counts *)p)->opens;
    return true;
}

static void resource_close(void *p, unsigned group, unsigned child)
{
    (void)group;
    (void)child;
    ++((struct msg_counts *)p)->closes;
}

static void callbacks(const char *mode, unsigned n)
{
    struct msg_counts counts = {0};
    struct msg_sample s[3] = {{0}};
    s[0].latency = calloc(n, sizeof(uint64_t));
    CHECK(s[0].latency);
    struct mfa_actor actor;
    struct mfs_tree tree;
    bool actors = !strcmp(mode, "actor_owned"),
         supervisors = !strcmp(mode, "supervisor_tree");
    if (actors)
        CHECK(mfa_init(&actor, value_handle, &counts, value_dead, &counts) ==
              MSG_OK);
    if (supervisors)
    {
        void *contexts[4][4];
        for (unsigned g = 0; g < 4; g++)
            for (unsigned c = 0; c < 4; c++)
                contexts[g][c] = &counts;
        CHECK(mfs_init(&tree, 4, 4, 1, resource_open, resource_close, &counts,
                       value_handle, contexts, value_dead, &counts) == MSG_OK);
    }
    uint64_t begin = now();
    for (unsigned i = 0; i < n; i++)
    {
        struct msg_value value = {i, &counts};
        struct mfo_value owner = {&value, value_drop};
        uint64_t start = now();
        if (actors)
        {
            CHECK(mfa_send(&actor, actor.actor.generation, &owner) == MSG_OK);
            CHECK(mfa_step(&actor) == MSG_OK);
        }
        else if (supervisors)
        {
            struct mfs_report report;
            CHECK(mfs_fault(&tree, 0, 2, (uint64_t)i * 10, MSG_REST, 0,
                            &report) == MSG_OK);
            CHECK(report.count == 4 && report.attempts == 2 &&
                  !report.parent_used && !report.rollback);
            s[0].checksum += report.count;
        }
        else
        {
            struct mfo_operation operation;
            CHECK(mfo_init(&operation, value_backend, &owner, value_receive,
                           &counts) == MSG_OK);
            CHECK(mfo_start(&operation) == MSG_OK);
        }
        if (!supervisors)
            CHECK(!owner.data && !owner.drop);
        ++s[0].success;
        record(&s[0], start);
    }
    uint64_t elapsed = now() - begin;
    if (actors)
        CHECK(mfa_destroy(&actor) == MSG_OK);
    if (supervisors)
    {
        CHECK(mfs_destroy(&tree) == MSG_OK);
        CHECK(counts.opens == 16 + 2 * n && counts.closes == counts.opens);
    }
    else
    {
        CHECK(counts.calls == n && counts.drops == n &&
              counts.checksum == (uint64_t)n * (n - 1) / 2);
        s[0].checksum = counts.checksum;
    }
    output(mode, s, elapsed, supervisors ? counts.opens : counts.drops);
    free(s[0].latency);
}

struct msg_stack
{
    atomic_uint *ready;
    size_t stack, guard;
};

static int stack_task(void *p, const atomic_bool *cancel)
{
    struct msg_stack *s = p;
    pthread_attr_t attr;
    CHECK(pthread_getattr_np(pthread_self(), &attr) == 0);
    CHECK(pthread_attr_getstacksize(&attr, &s->stack) == 0 &&
          pthread_attr_getguardsize(&attr, &s->guard) == 0);
    CHECK(pthread_attr_destroy(&attr) == 0);
    atomic_fetch_add_explicit(s->ready, 1, memory_order_release);
    while (!atomic_load_explicit(cancel, memory_order_acquire))
        sched_yield();
    return 0;
}

static void stacks(void)
{
    struct msg_nursery nursery;
    msg_nursery_init(&nursery, 0);
    atomic_uint ready;
    atomic_init(&ready, 0);
    struct msg_stack slots[8] = {0};
    uint64_t begin = now();
    for (unsigned i = 0; i < 8; i++)
    {
        slots[i].ready = &ready;
        CHECK(msg_spawn(&nursery, stack_task, &slots[i]) == MSG_OK);
    }
    while (atomic_load_explicit(&ready, memory_order_acquire) != 8)
        sched_yield();
    uint64_t spawn = now() - begin;
    msg_cancel(&nursery);
    begin = now();
    int result = 9;
    CHECK(msg_join(&nursery, &result) == MSG_OK && result == 0);
    uint64_t join = now() - begin;
    printf("{\"mode\":\"thread_stack\",\"spawn_to_ready_ns\":%" PRIu64
           ",\"cancel_to_join_ns\":%" PRIu64 ",\"stack_bytes\":[",
           spawn, join);
    for (unsigned i = 0; i < 8; i++)
    {
        if (i)
            putchar(',');
        printf("%zu", slots[i].stack);
    }
    printf("],\"guard_bytes\":[");
    for (unsigned i = 0; i < 8; i++)
    {
        if (i)
            putchar(',');
        printf("%zu", slots[i].guard);
    }
    puts("]}");
}

int main(int argc, char **argv)
{
    CHECK(argc == 3);
    unsigned n = (unsigned)strtoul(argv[2], NULL, 10);
    CHECK(n >= 8 && n <= 100000);
    const char *names[] = {
        "select_shared", "select_registered", "deque_sc",      "deque_weak",
        "deque_mutex",   "sequence_sc",       "sequence_weak", "sequence_mutex",
        "transaction",   "transaction_mutex"};
    if (!strcmp(argv[1], "representation"))
    {
        representation();
        return 0;
    }
    if (!strcmp(argv[1], "fairness"))
    {
        fairness(n);
        return 0;
    }
    if (!strcmp(argv[1], "thread_stack"))
    {
        stacks();
        return 0;
    }
    if (!strcmp(argv[1], "operation_owned") ||
        !strcmp(argv[1], "actor_owned") || !strcmp(argv[1], "supervisor_tree"))
    {
        callbacks(argv[1], n);
        return 0;
    }
    if (!strcmp(argv[1], "observer"))
    {
        struct msg_sample s[3] = {{0}};
        s[0].latency = calloc(n, sizeof(uint64_t));
        CHECK(s[0].latency);
        uint64_t begin = now();
        for (unsigned i = 0; i < n; i++)
        {
            uint64_t start = now();
            record(&s[0], start);
            ++s[0].success;
        }
        output("observer", s, now() - begin, 0);
        free(s[0].latency);
        return 0;
    }
    for (int i = 0; i < 10; i++)
        if (!strcmp(argv[1], names[i]))
        {
            threaded(names[i], i, n, i <= DEQUE_MUTEX ? 3 : 2);
            return 0;
        }
    const char *four[] = {"deque_sc4", "deque_weak4", "deque_mutex4"};
    for (int i = 0; i < 3; i++)
        if (!strcmp(argv[1], four[i]))
        {
            threaded(four[i], DEQUE_SC + i, n, 5);
            return 0;
        }
    if (!strcmp(argv[1], "snapshot_pins4") ||
        !strcmp(argv[1], "snapshot_copy4"))
    {
        snapshots_four(!strcmp(argv[1], "snapshot_pins4"), n);
        return 0;
    }
    if (!strcmp(argv[1], "snapshot_pins") || !strcmp(argv[1], "snapshot_copy"))
    {
        snapshots(!strcmp(argv[1], "snapshot_pins"), n);
        return 0;
    }
    if (!strcmp(argv[1], "stream_item") || !strcmp(argv[1], "stream_batch16"))
    {
        streams(!strcmp(argv[1], "stream_item") ? 1 : 16, n);
        return 0;
    }
    return 2;
}
