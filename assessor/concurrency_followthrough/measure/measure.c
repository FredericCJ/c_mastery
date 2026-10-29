#define _GNU_SOURCE
#include "measure.h"
#include "core.h"
#include "grace.h"
#include "reclamation.h"
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define CK(x)                                                          \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "CF_MEASURE_CHECK:%d:%s\n", __LINE__, #x); \
            exit(41);                                                  \
        }                                                              \
    } while (0)
#define LIMIT 40000u

static uint64_t now(void)
{
    struct timespec t;
    CK(!clock_gettime(CLOCK_MONOTONIC, &t));
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static void pause_owner(void)
{
    struct timespec t = {0, 2000000};
    while (nanosleep(&t, &t))
    {
    }
}

struct pool_actor
{
    struct cf_pool *pool;
    pthread_barrier_t *start;
    unsigned policy, worker, count;
    uint64_t scans, retries, elapsed;
};

static void *pool_actor(void *ptr)
{
    struct pool_actor *a = ptr;
    int b = pthread_barrier_wait(a->start);
    CK(!b || b == PTHREAD_BARRIER_SERIAL_THREAD);
    uint64_t start = now();
    for (unsigned i = 0; i < a->count; i++)
    {
        struct cf_ticket t;
        unsigned scans;
        int r;
        do
        {
            r = cf_take(a->pool, a->policy ? a->worker : 0, a->policy != 1, &t,
                        &scans);
            a->scans += scans;
            if (r == CF_FULL)
            {
                a->retries++;
                sched_yield();
            }
        } while (r == CF_FULL);
        CK(r == CF_OK);
        CK(cf_cancel(a->pool, t) == CF_OK);
    }
    a->elapsed = now() - start;
    return NULL;
}

int cf_pool_measure(unsigned policy, unsigned iterations, const char *csv)
{
    if (policy > 2 || !iterations || iterations > 100000 || !csv)
    {
        return 2;
    }
    struct cf_pool p;
    CK(cf_pool_init(&p, UINT_MAX >> 3) == CF_OK);
    unsigned admitted = 0, rejected = 0;
    struct cf_ticket held[24];
    for (unsigned worker = 0; worker < 4; worker++)
    {
        for (unsigned i = 0; i < (worker == 0 ? 12u : 4u); i++)
        {
            struct cf_ticket t;
            unsigned scans;
            int r = cf_take(&p, policy ? worker : 0, policy != 1, &t, &scans);
            if (r == CF_OK)
            {
                held[admitted++] = t;
            }
            else
            {
                CK(r == CF_FULL);
                rejected++;
            }
        }
    }
    CK(admitted == (policy == 1 ? 19u : 24u));
    CK(rejected == (policy == 1 ? 5u : 0u));
    for (unsigned i = 0; i < admitted; i++)
    {
        CK(cf_cancel(&p, held[i]) == CF_OK);
    }
    /* The skew test's released private reservations are externally quiescent;
     * initialize a distinct fresh object for the contention experiment. */
    struct cf_pool concurrent;
    CK(cf_pool_init(&concurrent, UINT_MAX >> 3) == CF_OK);
    pthread_barrier_t start;
    CK(!pthread_barrier_init(&start, NULL, 5));
    struct pool_actor a[4];
    pthread_t t[4];
    for (unsigned i = 0; i < 4; i++)
    {
        a[i] = (struct pool_actor){&concurrent, &start, policy, i,
                                   iterations,  0,      0,      0};
        CK(!pthread_create(&t[i], NULL, pool_actor, &a[i]));
    }
    uint64_t begin = now();
    int b = pthread_barrier_wait(&start);
    CK(!b || b == PTHREAD_BARRIER_SERIAL_THREAD);
    for (unsigned i = 0; i < 4; i++)
    {
        CK(!pthread_join(t[i], NULL));
    }
    uint64_t elapsed = now() - begin;
    for (unsigned i = 1; i < 32; i++)
    {
        CK((atomic_load(&concurrent.state[i]) & 7u) == CF_FREE);
    }
    FILE *f = fopen(csv, "w");
    CK(f);
    fputs("worker,completed,scans,retries,elapsed_ns\n", f);
    for (unsigned i = 0; i < 4; i++)
    {
        fprintf(f, "%u,%u,%llu,%llu,%llu\n", i, iterations,
                (unsigned long long)a[i].scans,
                (unsigned long long)a[i].retries,
                (unsigned long long)a[i].elapsed);
    }
    CK(!fclose(f));
    printf(
        "CF_MEASURE pool policy=%u iterations=%u participants=4 slots=32 dummy=1 "
        "skew_demand=24 admitted=%u rejected=%u idle_free=%u elapsed_ns=%llu\n",
        policy, iterations, admitted, rejected, 31 - admitted,
        (unsigned long long)elapsed);
    CK(!pthread_barrier_destroy(&start));
    return 0;
}

struct cf_locked
{
    pthread_mutex_t lock;
    pthread_cond_t readable, writable;
    struct cf_value values[31];
    unsigned head, count;
};

struct queue_run
{
    struct cf_ring ring;
    struct ap_queue ap;
    struct cf_locked mutex;
    pthread_barrier_t start;
    unsigned kind, pairs, bytes, affinity, pause, total, cpu[2], ncpus;
    unsigned char payload[LIMIT][256];
    uint64_t started[LIMIT], push_latency[LIMIT], latency[LIMIT];
    unsigned receiver[LIMIT];
    atomic_uint seen[LIMIT];
};
static struct queue_run qr;

struct q_actor
{
    struct queue_run *q;
    unsigned role;
    struct ap_lease lease;
    uint64_t retries, completed;
    unsigned held_retired;
};

static void pin(struct queue_run *q, unsigned role)
{
    if (!q->affinity)
    {
        return;
    }
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(q->cpu[q->affinity == 1 ? 0 : role % q->ncpus], &set);
    CK(!pthread_setaffinity_np(pthread_self(), sizeof set, &set));
}

static void locked_push(struct cf_locked *q, struct cf_value v)
{
    CK(!pthread_mutex_lock(&q->lock));
    while (q->count == 31)
    {
        CK(!pthread_cond_wait(&q->writable, &q->lock));
    }
    q->values[(q->head + q->count) % 31] = v;
    q->count++;
    CK(!pthread_cond_signal(&q->readable));
    CK(!pthread_mutex_unlock(&q->lock));
}

static struct cf_value locked_pop(struct cf_locked *q)
{
    CK(!pthread_mutex_lock(&q->lock));
    while (!q->count)
    {
        CK(!pthread_cond_wait(&q->readable, &q->lock));
    }
    struct cf_value v = q->values[q->head];
    q->head = (q->head + 1) % 31;
    q->count--;
    CK(!pthread_cond_signal(&q->writable));
    CK(!pthread_mutex_unlock(&q->lock));
    return v;
}

static int locked_try(struct cf_locked *q, struct cf_value *v, unsigned push)
{
    CK(!pthread_mutex_lock(&q->lock));
    int result = CF_OK;
    if (push)
    {
        if (q->count == 31)
        {
            result = CF_FULL;
        }
        else
        {
            q->values[(q->head + q->count) % 31] = *v;
            q->count++;
        }
    }
    else if (!q->count)
    {
        result = CF_EMPTY;
    }
    else
    {
        *v = q->values[q->head];
        q->head = (q->head + 1) % 31;
        q->count--;
    }
    CK(!pthread_mutex_unlock(&q->lock));
    return result;
}

static void *queue_actor(void *ptr)
{
    struct q_actor *a = ptr;
    struct queue_run *q = a->q;
    pin(q, a->role);
    unsigned hold = q->pause && a->role == q->pairs;
    if (hold && q->kind == 1)
    {
        atomic_store(&q->ap.hazard[a->lease.participant][0], 0);
    }
    int barrier = pthread_barrier_wait(&q->start);
    CK(barrier == 0 || barrier == PTHREAD_BARRIER_SERIAL_THREAD);
    if (hold)
    {
        pause_owner();
        if (q->kind == 1)
        {
            for (unsigned i = 0; i < 32; i++)
            {
                a->held_retired +=
                    (atomic_load(&q->ap.node[i].state) & 3u) == 2;
            }
            atomic_store(&q->ap.hazard[a->lease.participant][0], AP_NONE);
        }
    }
    unsigned count = q->total / q->pairs;
    for (unsigned i = 0; i < count; i++)
    {
        if (a->role < q->pairs)
        {
            unsigned id = a->role * count + i;
            struct cf_value v = {id, ~id};
            uint64_t start = now();
            q->started[id] = start;
            if (q->kind == 0)
            {
                int r;
                while ((r = cf_push(&q->ring, &v, 1, NULL, NULL)) == CF_FULL)
                {
                    a->retries++;
                    sched_yield();
                }
                CK(r == CF_OK);
            }
            else if (q->kind == 1)
            {
                int r;
                while ((r = ap_push(&q->ap, a->lease,
                                    (struct ap_value){id, ~id})) == AP_FULL)
                {
                    a->retries++;
                    ap_collect(&q->ap);
                    sched_yield();
                }
                CK(r == AP_OK);
            }
            else if (q->kind == 2)
            {
                locked_push(&q->mutex, v);
            }
            else
            {
                int r;
                while ((r = locked_try(&q->mutex, &v, 1)) == CF_FULL)
                {
                    a->retries++;
                    sched_yield();
                }
                CK(r == CF_OK);
            }
            q->push_latency[id] = now() - start;
        }
        else
        {
            struct cf_value v;
            if (q->kind == 0)
            {
                int r;
                while ((r = cf_pop(&q->ring, &v, 1, NULL, NULL)) == CF_EMPTY)
                {
                    a->retries++;
                    sched_yield();
                }
                CK(r == CF_OK);
            }
            else if (q->kind == 1)
            {
                struct ap_value value;
                int r;
                while ((r = ap_pop(&q->ap, a->lease, &value)) == AP_RETRY)
                {
                    a->retries++;
                    ap_collect(&q->ap);
                    sched_yield();
                }
                CK(r == AP_OK);
                v = (struct cf_value){value.sequence, value.inverse};
                ap_collect(&q->ap);
            }
            else if (q->kind == 2)
            {
                v = locked_pop(&q->mutex);
            }
            else
            {
                int r;
                while ((r = locked_try(&q->mutex, &v, 0)) == CF_EMPTY)
                {
                    a->retries++;
                    sched_yield();
                }
                CK(r == CF_OK);
            }
            CK(v.sequence < q->total && v.inverse == ~v.sequence);
            for (unsigned b = 0; b < q->bytes; b++)
            {
                CK(q->payload[v.sequence][b] ==
                   (unsigned char)(v.sequence + b));
            }
            CK(atomic_fetch_add_explicit(&q->seen[v.sequence], 1,
                                         memory_order_relaxed) == 0);
            q->receiver[v.sequence] = a->role;
            q->latency[v.sequence] = now() - q->started[v.sequence];
        }
        a->completed++;
    }
    if (q->kind == 1)
    {
        CK(ap_leave(&q->ap, a->lease) == AP_OK);
    }
    return NULL;
}

int cf_queue_measure(unsigned kind, unsigned pairs, unsigned bytes,
                     unsigned affinity, unsigned pause, unsigned total,
                     const char *csv)
{
    if (kind > 3 || (pairs != 1 && pairs != 2 && pairs != 4) ||
        (kind == 0 && pairs != 1) ||
        (bytes != 8 && bytes != 64 && bytes != 256) || affinity > 2 ||
        pause > 1 || total > LIMIT || !total || total % pairs || !csv)
    {
        return 2;
    }
    struct queue_run *q = &qr;
    q->kind = kind;
    q->pairs = pairs;
    q->bytes = bytes;
    q->affinity = affinity;
    q->pause = pause;
    q->total = total;
    q->ncpus = 0;
    cpu_set_t allowed;
    CK(!sched_getaffinity(0, sizeof allowed, &allowed));
    for (unsigned i = 0; i < CPU_SETSIZE && q->ncpus < 2; i++)
    {
        if (CPU_ISSET(i, &allowed))
        {
            q->cpu[q->ncpus++] = i;
        }
    }
    CK(q->ncpus);
    if (affinity == 2 && q->ncpus < 2)
    {
        return 3;
    }
    CK(cf_ring_init(&q->ring, 32, 31, 32) == CF_OK);
    CK(ap_init(&q->ap, 2 * pairs, UINT_MAX >> 2, NULL, NULL) == AP_OK);
    CK(!pthread_mutex_init(&q->mutex.lock, NULL));
    CK(!pthread_cond_init(&q->mutex.readable, NULL));
    CK(!pthread_cond_init(&q->mutex.writable, NULL));
    q->mutex.head = q->mutex.count = 0;
    CK(!pthread_barrier_init(&q->start, NULL, 2 * pairs + 1));
    for (unsigned id = 0; id < total; id++)
    {
        atomic_init(&q->seen[id], 0);
        for (unsigned b = 0; b < bytes; b++)
        {
            q->payload[id][b] = (unsigned char)(id + b);
        }
    }
    struct q_actor actors[8];
    pthread_t t[8];
    for (unsigned i = 0; i < 2 * pairs; i++)
    {
        struct ap_lease l;
        CK(ap_register(&q->ap, &l) == AP_OK);
        actors[i] = (struct q_actor){q, i, l, 0, 0, 0};
        CK(!pthread_create(&t[i], NULL, queue_actor, &actors[i]));
    }
    uint64_t start = now();
    int barrier = pthread_barrier_wait(&q->start);
    CK(barrier == 0 || barrier == PTHREAD_BARRIER_SERIAL_THREAD);
    for (unsigned i = 0; i < 2 * pairs; i++)
    {
        CK(!pthread_join(t[i], NULL));
    }
    uint64_t elapsed = now() - start;
    FILE *f = fopen(csv, "w");
    CK(f);
    fputs("id,producer,consumer,enqueue_ns,completion_ns\n", f);
    for (unsigned id = 0; id < total; id++)
    {
        CK(atomic_load(&q->seen[id]) == 1);
        fprintf(f, "%u,%u,%u,%llu,%llu\n", id, id / (total / pairs),
                q->receiver[id], (unsigned long long)q->push_latency[id],
                (unsigned long long)q->latency[id]);
    }
    CK(!fclose(f));
    uint64_t retries = 0;
    for (unsigned i = 0; i < 2 * pairs; i++)
    {
        CK(actors[i].completed == total / pairs);
        retries += actors[i].retries;
        printf(
            "CF_ACTOR role=%u completed=%llu retries=%llu retained_at_resume=%u\n",
            i, (unsigned long long)actors[i].completed,
            (unsigned long long)actors[i].retries, actors[i].held_retired);
    }
    printf("CF_MEASURE queue kind=%u pairs=%u bytes=%u affinity=%u cpu0=%u "
           "cpu1=%u pause=%u total=%u elapsed_ns=%llu retries=%llu "
           "ring_bytes=%zu ap_bytes=%zu mutex_bytes=%zu payload_storage=%zu "
           "padded=%u capacity_upper=31 atomic_lock_free=%d\n",
           kind, pairs, bytes, affinity, q->cpu[0], q->cpu[q->ncpus - 1], pause,
           total, (unsigned long long)elapsed, (unsigned long long)retries,
           sizeof(struct cf_ring), sizeof(struct ap_queue),
           sizeof(struct cf_locked), sizeof(q->payload), CF_PAD,
           atomic_is_lock_free(&q->ap.head));
    CK(!pthread_barrier_destroy(&q->start));
    CK(!pthread_mutex_destroy(&q->mutex.lock));
    CK(!pthread_cond_destroy(&q->mutex.readable));
    CK(!pthread_cond_destroy(&q->mutex.writable));
    return 0;
}

struct rw_run
{
    struct ag_store store;
    pthread_mutex_t lock;
    pthread_rwlock_t rw;
    pthread_barrier_t start;
    unsigned char rw_storage[17][32];
    unsigned kind, pause, writes;
    uint64_t reader_ns[2][4000], writer_ns[4000];
    unsigned retries, peak_retained;
    struct ag_lease lease[2];
    atomic_uint paused_ready;
};
static struct rw_run rr;

struct rw_actor
{
    struct rw_run *r;
    unsigned role;
};

static void *rw_actor(void *ptr)
{
    struct rw_actor *a = ptr;
    struct rw_run *r = a->r;
    int gate = pthread_barrier_wait(&r->start);
    CK(!gate || gate == PTHREAD_BARRIER_SERIAL_THREAD);
    if (a->role == 2 && r->pause)
    {
        while (!atomic_load_explicit(&r->paused_ready, memory_order_acquire))
        {
            sched_yield();
        }
    }
    if (a->role == 2)
    {
        for (unsigned i = 0; i < r->writes; i++)
        {
            uint64_t start = now();
            if (r->kind == 0)
            {
                int status;
                do
                {
                    CK(!pthread_mutex_lock(&r->lock));
                    CK(ag_writer_acquire(&r->store, 7) == AG_OK);
                    status = ag_publish(&r->store, 7, (unsigned char)i);
                    CK(status == AG_OK || status == AG_FULL);
                    unsigned retained = 0;
                    for (unsigned j = 0; j < 17; j++)
                    {
                        retained += r->store.objects[j].active &&
                                    r->store.objects[j].retired;
                    }
                    if (retained > r->peak_retained)
                    {
                        r->peak_retained = retained;
                    }
                    ag_collect(&r->store);
                    CK(ag_writer_release(&r->store, 7) == AG_OK);
                    CK(!pthread_mutex_unlock(&r->lock));
                    if (status == AG_FULL)
                    {
                        r->retries++;
                        sched_yield();
                    }
                } while (status == AG_FULL);
            }
            else
            {
                CK(!pthread_rwlock_wrlock(&r->rw));
                for (unsigned j = 0; j < 32; j++)
                {
                    r->rw_storage[0][j] = (unsigned char)i;
                }
                CK(!pthread_rwlock_unlock(&r->rw));
            }
            r->writer_ns[i] = now() - start;
        }
    }
    else
    {
        for (unsigned i = 0; i < r->writes; i++)
        {
            uint64_t start = now();
            unsigned char bytes[32];
            if (r->kind == 0)
            {
                struct ag_borrow borrow;
                CK(!pthread_mutex_lock(&r->lock));
                CK(ag_read(&r->store, r->lease[a->role], &borrow) == AG_OK);
                CK(ag_copy(&r->store, borrow, bytes) == AG_OK);
                CK(!pthread_mutex_unlock(&r->lock));
                if (r->pause && !i && !a->role)
                {
                    atomic_store_explicit(&r->paused_ready, 1,
                                          memory_order_release);
                    pause_owner();
                }
                CK(!pthread_mutex_lock(&r->lock));
                CK(ag_read_end(&r->store, borrow) == AG_OK);
                ag_collect(&r->store);
                CK(!pthread_mutex_unlock(&r->lock));
            }
            else
            {
                CK(!pthread_rwlock_rdlock(&r->rw));
                for (unsigned j = 0; j < 32; j++)
                {
                    bytes[j] = r->rw_storage[0][j];
                }
                if (r->pause && !i && !a->role)
                {
                    atomic_store_explicit(&r->paused_ready, 1,
                                          memory_order_release);
                    pause_owner();
                }
                CK(!pthread_rwlock_unlock(&r->rw));
            }
            for (unsigned j = 1; j < 32; j++)
            {
                CK(bytes[j] == bytes[0]);
            }
            r->reader_ns[a->role][i] = now() - start;
        }
    }
    return NULL;
}

int cf_rw_measure(unsigned kind, unsigned pause, unsigned writes,
                  const char *csv)
{
    if (kind > 1 || pause > 1 || !writes || writes > 4000 || !csv)
    {
        return 2;
    }
    struct rw_run *r = &rr;
    r->kind = kind;
    r->pause = pause;
    r->writes = writes;
    r->retries = r->peak_retained = 0;
    atomic_init(&r->paused_ready, 0);
    ag_store_init(&r->store, 0, 0);
    for (unsigned i = 0; i < 2; i++)
    {
        CK(ag_register(&r->store.domain, &r->lease[i]) == AG_OK);
    }
    for (unsigned i = 0; i < 17; i++)
    {
        for (unsigned b = 0; b < 32; b++)
        {
            r->rw_storage[i][b] = 0;
        }
    }
    CK(!pthread_mutex_init(&r->lock, NULL));
    CK(!pthread_rwlock_init(&r->rw, NULL));
    CK(!pthread_barrier_init(&r->start, NULL, 4));
    struct rw_actor a[3];
    pthread_t t[3];
    for (unsigned i = 0; i < 3; i++)
    {
        a[i] = (struct rw_actor){r, i};
        CK(!pthread_create(&t[i], NULL, rw_actor, &a[i]));
    }
    uint64_t start = now();
    int gate = pthread_barrier_wait(&r->start);
    CK(!gate || gate == PTHREAD_BARRIER_SERIAL_THREAD);
    for (unsigned i = 0; i < 3; i++)
    {
        CK(!pthread_join(t[i], NULL));
    }
    uint64_t elapsed = now() - start;
    FILE *f = fopen(csv, "w");
    CK(f);
    fputs("role,operation,latency_ns\n", f);
    for (unsigned i = 0; i < writes; i++)
    {
        fprintf(f, "writer,%u,%llu\n", i, (unsigned long long)r->writer_ns[i]);
        for (unsigned j = 0; j < 2; j++)
        {
            fprintf(f, "reader%u,%u,%llu\n", j, i,
                    (unsigned long long)r->reader_ns[j][i]);
        }
    }
    CK(!fclose(f));
    printf(
        "CF_MEASURE rw kind=%u pause=%u readers=2 writes=%u payload=32 "
        "version_capacity=17 elapsed_ns=%llu full_retries=%u peak_retired=%u "
        "ag_bytes=%zu rw_payload_bytes=%zu\n",
        kind, pause, writes, (unsigned long long)elapsed, r->retries,
        r->peak_retained, sizeof(struct ag_store), sizeof(r->rw_storage));
    for (unsigned i = 0; i < 2; i++)
    {
        CK(ag_depart(&r->store.domain, r->lease[i]) == AG_OK);
    }
    CK(!pthread_barrier_destroy(&r->start));
    CK(!pthread_rwlock_destroy(&r->rw));
    CK(!pthread_mutex_destroy(&r->lock));
    return 0;
}
