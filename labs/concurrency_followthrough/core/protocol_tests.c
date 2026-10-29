#define _POSIX_C_SOURCE 200809L
#include "cmx_advanced.h"
#include "history.h"
#include "reclamation.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CK(x)                                                  \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CP_CHECK:%d:%s\n", __LINE__, #x); \
            exit(41);                                          \
        }                                                      \
    } while (0)

struct cf_gate
{
    pthread_mutex_t lock;
    pthread_cond_t cv;
    unsigned point, node, entered, release;
};

static _Thread_local unsigned gated;

static void hook(void *ctx, unsigned point, unsigned participant, unsigned node)
{
    (void)participant;
    struct cf_gate *g = ctx;
    if (!gated || point != g->point || node != g->node)
    {
        return;
    }
    CK(!pthread_mutex_lock(&g->lock));
    if (!g->entered)
    {
        g->entered = 1;
        CK(!pthread_cond_broadcast(&g->cv));
        while (!g->release)
        {
            CK(!pthread_cond_wait(&g->cv, &g->lock));
        }
    }
    CK(!pthread_mutex_unlock(&g->lock));
}

struct cf_operation
{
    struct ap_queue *q;
    struct ap_lease l;
    struct ch_op *o;
    atomic_uint *clock;
    unsigned push;
    struct ap_value out;
    int result;
};

static void record(struct cf_operation *a, unsigned role)
{
    gated = role;
    struct ch_op *o = a->o;
    o->participant = a->l.participant;
    o->operation = a->push ? CH_INSERT : CH_REMOVE;
    o->pending = 0;
    o->invoke = atomic_fetch_add(a->clock, 1);
    o->invoke_event = o->invoke;
    struct ap_value v = {11, ~11u};
    a->result = a->push ? ap_push(a->q, a->l, v) : ap_pop(a->q, a->l, &a->out);
    CK(a->result == AP_OK);
    o->value = a->push ? 11 : a->out.sequence;
    o->result = CH_VALUE;
    o->respond = atomic_fetch_add(a->clock, 1);
    o->respond_event = o->respond;
}

static void *operate(void *p)
{
    record(p, 1);
    return NULL;
}

static void pause_cases(void)
{
    for (unsigned push = 0; push < 2; push++)
    {
        struct cf_gate g = {PTHREAD_MUTEX_INITIALIZER,
                            PTHREAD_COND_INITIALIZER,
                            push ? AP_LINKED : AP_VALIDATED,
                            push ? 1u : 0u,
                            0,
                            0};
        struct ap_queue q;
        struct ap_lease slow, actor;
        CK(ap_init(&q, 2, 100, hook, &g) == AP_OK);
        CK(ap_register(&q, &slow) == AP_OK && ap_register(&q, &actor) == AP_OK);
        struct ch_op ops[5] = {0};
        unsigned n = 0;
        atomic_uint clock;
        atomic_init(&clock, 0);
        if (!push)
        {
            for (unsigned i = 0; i < 2; i++)
            {
                unsigned v = 11 * (i + 1);
                ops[n] = (struct ch_op){2 * i,     2 * i + 1,         2 * i,
                                        2 * i + 1, actor.participant, CH_INSERT,
                                        v,         CH_VALUE,          0};
                CK(ap_push(&q, actor, (struct ap_value){v, ~v}) == AP_OK);
                n++;
                atomic_store(&clock, 2 * (i + 1));
            }
        }
        struct cf_operation a = {&q, slow, &ops[n], &clock, push, {0, 0}, -1};
        n++;
        pthread_t t;
        CK(!pthread_create(&t, NULL, operate, &a));
        CK(!pthread_mutex_lock(&g.lock));
        while (!g.entered)
        {
            CK(!pthread_cond_wait(&g.cv, &g.lock));
        }
        CK(!pthread_mutex_unlock(&g.lock));
        if (push)
        {
            CK(atomic_load(&q.tail) == 0);
            struct ch_op *o = &ops[n++];
            o->participant = actor.participant;
            o->operation = CH_INSERT;
            o->value = 22;
            o->result = CH_VALUE;
            o->invoke = atomic_fetch_add(&clock, 1);
            o->invoke_event = o->invoke;
            CK(ap_push(&q, actor, (struct ap_value){22, ~22u}) == AP_OK);
            o->respond = atomic_fetch_add(&clock, 1);
            o->respond_event = o->respond;
        }
        struct cf_operation b = {&q, actor, &ops[n++], &clock, 0, {0, 0}, -1};
        record(&b, 0);
        CK(b.out.sequence == 11 && b.out.inverse == ~11u);
        if (!push)
        {
            CK(ap_collect(&q) == 0 &&
               (atomic_load(&q.node[0].state) & 3u) == 2);
        }
        CK(!pthread_mutex_lock(&g.lock));
        g.release = 1;
        CK(!pthread_cond_broadcast(&g.cv));
        CK(!pthread_mutex_unlock(&g.lock));
        CK(!pthread_join(t, NULL));
        CK(a.result == AP_OK);
        if (!push)
        {
            CK(a.out.sequence == 22 && a.out.inverse == ~22u);
        }
        struct ch_witness w;
        uint32_t visited;
        CK(ch_check(ops, n, 4, CH_FIFO, 0, 1, 100000, &w, &visited) ==
           CH_VALID);
        printf("CP_HISTORY %u", push);
        for (unsigned i = 0; i < n; i++)
        {
            printf(" %u,%u,%u,%u,%u,%u,%u,%u,%u", ops[i].invoke, ops[i].respond,
                   ops[i].invoke_event, ops[i].respond_event,
                   ops[i].participant, ops[i].operation, ops[i].value,
                   ops[i].result, ops[i].pending);
        }
        puts("");
        CK(!pthread_cond_destroy(&g.cv));
        CK(!pthread_mutex_destroy(&g.lock));
    }
    puts("CP_PASS:pause actual postlink enqueue+dequeue help and overlapping "
         "two-dequeuer FIFO witnesses");
}

struct cf_worker
{
    struct ap_queue *q;
    struct ap_lease l;
    unsigned role;
    atomic_uint *seen, *received;
};

static void *stress(void *ptr)
{
    struct cf_worker *w = ptr;
    gated = 0;
    if (w->role < 2)
    {
        for (unsigned i = 0; i < 20000; i++)
        {
            unsigned id = w->role * 20000 + i;
            int r;
            while ((r = ap_push(w->q, w->l, (struct ap_value){id, ~id})) ==
                   AP_FULL)
            {
                ap_collect(w->q);
                sched_yield();
            }
            CK(r == AP_OK);
        }
    }
    else
    {
        while (atomic_load(w->received) < 40000)
        {
            struct ap_value v;
            int r = ap_pop(w->q, w->l, &v);
            if (r == AP_RETRY)
            {
                ap_collect(w->q);
                sched_yield();
                continue;
            }
            CK(r == AP_OK && v.sequence < 40000 && v.inverse == ~v.sequence);
            CK(atomic_fetch_add(&w->seen[v.sequence], 1) == 0);
            atomic_fetch_add(w->received, 1);
            ap_collect(w->q);
        }
    }
    CK(ap_leave(w->q, w->l) == AP_OK);
    return NULL;
}

static void multiplicity(void)
{
    struct ap_queue q;
    CK(ap_init(&q, 4, UINT_MAX >> 2, NULL, NULL) == AP_OK);
    static atomic_uint seen[40000];
    atomic_uint received;
    atomic_init(&received, 0);
    for (unsigned i = 0; i < 40000; i++)
    {
        atomic_init(&seen[i], 0);
    }
    struct cf_worker a[4];
    pthread_t t[4];
    for (unsigned i = 0; i < 4; i++)
    {
        struct ap_lease l;
        CK(ap_register(&q, &l) == AP_OK);
        a[i] = (struct cf_worker){&q, l, i, seen, &received};
        CK(!pthread_create(&t[i], NULL, stress, &a[i]));
    }
    for (unsigned i = 0; i < 4; i++)
    {
        CK(!pthread_join(t[i], NULL));
    }
    for (unsigned i = 0; i < 40000; i++)
    {
        CK(atomic_load(&seen[i]) == 1);
    }
    CK(atomic_load(&received) == 40000);
    ap_collect(&q);
    unsigned live = 0;
    for (unsigned i = 0; i < 32; i++)
    {
        live += (atomic_load(&q.node[i].state) & 3u) == 1;
    }
    CK(live == 1);
    puts("CP_PASS:stress values=40000 distinct_once=40000 threads=4 joined=4 "
         "remaining_dummy=1 finite_not_general_FIFO_or_progress");
}

static void baseline(void)
{
    cmx_reservation q;
    cmx_reservation_init(&q);
    uint32_t zero, one, out = 99;
    CK(cmx_reserve(&q, &zero) == CMX_OK && zero == 0);
    CK(cmx_reserve(&q, &one) == CMX_OK && one == 1);
    CK(cmx_publish(&q, one, 22) == CMX_OK);
    CK(cmx_reserved_pop(&q, &out) == CMX_RETRY && out == 99);
    CK(cmx_publish(&q, zero, 11) == CMX_OK);
    CK(cmx_reserved_pop(&q, &out) == CMX_OK && out == 11);
    CK(cmx_reserved_pop(&q, &out) == CMX_OK && out == 22);
    cmx_history_op h[9] = {{0, 0, 0, 1, 7, CMX_OK},
                           {1, 0, 2, 3, 8, CMX_OK},
                           {2, 1, 4, 5, 8, CMX_OK}};
    unsigned w[9];
    CK(cmx_history_check(h, 3, 4, w) == CMX_BROKEN);
    h[2].value = 7;
    CK(cmx_history_check(h, 3, 4, w) == CMX_OK);
    h[0].respond = h[0].invoke;
    CK(cmx_history_check(h, 3, 4, w) == CMX_INVALID);
    CK(cmx_history_check(h, 9, 4, w) == CMX_INVALID);
    cmx_identity_pool p;
    cmx_identity_init(&p);
    for (unsigned generation = 1; generation <= 65535; generation++)
    {
        uint32_t id = cmx_identity_take(&p);
        CK((id & 255u) == 1 && id >> 8 == generation);
        CK(cmx_identity_put(&p, id) == CMX_OK && !cmx_identity_valid(&p, id));
    }
    CK((cmx_identity_take(&p) & 255u) == 2);
    puts("CP_PASS:baseline reservation0_blocks_published1 "
         "original_complete_history_limit8 identity_slot_retires65535");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "pause"))
    {
        pause_cases();
    }
    else if (!strcmp(argv[1], "stress"))
    {
        multiplicity();
    }
    else if (!strcmp(argv[1], "baseline"))
    {
        baseline();
    }
    else
    {
        return 2;
    }
    return 0;
}
