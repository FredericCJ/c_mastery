#define _POSIX_C_SOURCE 200809L
#include "cmx_advanced.h"
#include "epoch.h"
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
            fprintf(stderr, "CE_CHECK:%d:%s\n", __LINE__, #x); \
            exit(41);                                          \
        }                                                      \
    } while (0)

struct cf_actor
{
    struct cf_epoch *e;
    struct ag_lease lease;
    unsigned loops;
    atomic_uint *done;
};

static void *actor(void *p)
{
    struct cf_actor *a = p;
    for (unsigned i = 0; i < a->loops; i++)
    {
        int r;
        while ((r = cf_epoch_enter(a->e, a->lease)) == AG_BUSY)
            sched_yield();
        CK(r == AG_OK);
        while ((r = cf_epoch_leave(a->e, a->lease)) == AG_BUSY)
            sched_yield();
        CK(r == AG_OK);
    }
    atomic_fetch_add(a->done, 1);
    return NULL;
}

static void registration(void)
{
    struct cf_epoch e;
    cf_epoch_init(&e, 1);
    struct ag_lease a, b;
    CK(cf_epoch_register(&e, &a) == AG_OK);
    CK(cf_epoch_enter(&e, a) == AG_OK && cf_epoch_enter(&e, a) == AG_OK);
    CK(cf_epoch_advance(&e) == AG_OK && e.domain.retained.epoch == 1);
    CK(cf_epoch_advance(&e) == AG_PENDING && cf_epoch_depart(&e, a) == AG_BUSY);
    unsigned eligible = 99;
    CK(cf_epoch_eligible(&e, 0, &eligible) == AG_OK && !eligible);
    CK(cf_epoch_leave(&e, a) == AG_OK);
    CK(cf_epoch_eligible(&e, 0, &eligible) == AG_OK && !eligible);
    CK(cf_epoch_leave(&e, a) == AG_OK);
    CK(cf_epoch_eligible(&e, 0, &eligible) == AG_OK && eligible);
    CK(cf_epoch_depart(&e, a) == AG_OK && cf_epoch_register(&e, &b) == AG_OK &&
       b.generation == a.generation + 1);
    CK(cf_epoch_enter(&e, a) == AG_INVALID);
    /* Quiescent boundary injection; old registration cannot survive reset. */
    CK(cf_epoch_depart(&e, b) == AG_OK);
    e.domain.generation[0] = UINT_MAX - 1;
    CK(cf_epoch_register(&e, &a) == AG_OK && a.participant == 0);
    CK(cf_epoch_depart(&e, a) == AG_OK && e.domain.generation[0] == UINT_MAX);
    CK(cf_epoch_register(&e, &b) == AG_OK && b.participant == 1);
    CK(!atomic_flag_test_and_set_explicit(&e.gate, memory_order_acquire));
    eligible = 99;
    CK(cf_epoch_eligible(&e, 0, &eligible) == AG_BUSY && eligible == 99);
    atomic_flag_clear_explicit(&e.gate, memory_order_release);
    puts("CE_PASS:epoch0 advance1 pending2 nested_outermost ticket_retirement "
         "stale_rejection busy_preserved");
}

static void threads(void)
{
    struct cf_epoch e;
    cf_epoch_init(&e, 0);
    struct cf_actor a[2];
    pthread_t t[2];
    atomic_uint done;
    atomic_init(&done, 0);
    for (unsigned i = 0; i < 2; i++)
    {
        struct ag_lease l;
        CK(cf_epoch_register(&e, &l) == AG_OK);
        a[i] = (struct cf_actor){&e, l, 10000, &done};
    }
    for (unsigned i = 0; i < 2; i++)
    {
        CK(!pthread_create(&t[i], NULL, actor, &a[i]));
    }
    unsigned advances = 0, pending = 0, busy = 0;
    while (atomic_load(&done) != 2)
    {
        int r = cf_epoch_advance(&e);
        if (r == AG_OK)
            advances++;
        else if (r == AG_PENDING)
            pending++;
        else
        {
            CK(r == AG_BUSY);
            busy++;
        }
    }
    for (unsigned i = 0; i < 2; i++)
    {
        CK(!pthread_join(t[i], NULL));
        CK(cf_epoch_depart(&e, a[i].lease) == AG_OK);
    }
    printf(
        "CE_PASS:threads read_sections=20000 joined=2 advances=%u pending=%u "
        "busy=%u\n",
        advances, pending, busy);
}

static void retention(void)
{
    for (unsigned period = 3; period <= 12; period *= 2)
    {
        struct ag_store s;
        struct ag_lease reader;
        struct ag_borrow borrow;
        ag_store_init(&s, 0, 0);
        CK(ag_register(&s.domain, &reader) == AG_OK);
        CK(ag_read(&s, reader, &borrow) == AG_OK);
        CK(ag_writer_acquire(&s, 7) == AG_OK);
        struct ap_queue q;
        struct ap_lease hold, actor;
        CK(ap_init(&q, 2, 100, NULL, NULL) == AP_OK);
        CK(ap_register(&q, &hold) == AP_OK);
        CK(ap_register(&q, &actor) == AP_OK);
        atomic_store(&q.hazard[hold.participant][0], atomic_load(&q.head));
        unsigned peak_epoch = 0, peak_hazard = 0;
        for (unsigned i = 1; i <= 12; i++)
        {
            unsigned retired_epoch = s.domain.retained.epoch;
            CK(ag_publish(&s, 7, (unsigned char)i) == AG_OK);
            CK(ag_collect(&s) == 0);
            struct ap_value out, v = {i, ~i};
            CK(ap_push(&q, actor, v) == AP_OK &&
               ap_pop(&q, actor, &out) == AP_OK && out.sequence == i);
            ap_collect(&q);
            unsigned ecount = 0, hcount = 0;
            for (unsigned j = 0; j < 17; j++)
                ecount += s.objects[j].active && s.objects[j].retired;
            for (unsigned j = 0; j < 32; j++)
                hcount += (atomic_load(&q.node[j].state) & 3u) == 2;
            if (ecount > peak_epoch)
                peak_epoch = ecount;
            if (hcount > peak_hazard)
                peak_hazard = hcount;
            printf(
                "CE_RETIRE period=%u operation=%u epoch=%u epoch_retained=%u "
                "hazard_retained=%u earliest_quiescent_operation=%u\n",
                period, i, retired_epoch, ecount, hcount,
                ((i - 1) / period + 1) * period);
            if (i % period == 0)
            {
                CK(ag_read_end(&s, borrow) == AG_OK);
                CK(ag_collect(&s) == period);
                atomic_store(&q.hazard[hold.participant][0], AP_NONE);
                CK(ap_collect(&q) == 1);
                if (i < 12)
                {
                    CK(ag_read(&s, reader, &borrow) == AG_OK);
                    atomic_store(&q.hazard[hold.participant][0],
                                 atomic_load(&q.head));
                }
            }
        }
        CK(peak_epoch == period && peak_hazard == 1);
        printf("CE_PASS:retention period=%u epoch_peak=%u hazard_peak=%u "
               "same_retirements=12 metadata_not_equal=1\n",
               period, peak_epoch, peak_hazard);
    }
}

static void arrivals(void)
{
    struct ag_store s;
    struct ag_lease old, newer[2];
    struct ag_borrow a, b, next;
    ag_store_init(&s, 1, 1);
    CK(ag_register(&s.domain, &old) == AG_OK);
    for (unsigned i = 0; i < 2; i++)
        CK(ag_register(&s.domain, &newer[i]) == AG_OK);
    CK(ag_read(&s, old, &a) == AG_OK);
    CK(ag_writer_acquire(&s, 7) == AG_OK);
    CK(ag_publish(&s, 7, 2) == AG_OK);
    CK(ag_read(&s, newer[0], &b) == AG_OK);
    CK(ag_read_end(&s, a) == AG_OK);
    CK(ag_collect(&s) == 1);
    cmx_rcu r;
    unsigned root;
    cmx_rcu_init(&r, 2);
    CK(cmx_rcu_read(&r, 0, &root) == CMX_OK);
    for (unsigned i = 0; i < 1000; i++)
    {
        unsigned active = i % 2, fresh = 1 - active;
        CK(ag_read(&s, newer[fresh], &next) == AG_OK);
        CK(ag_read_end(&s, b) == AG_OK);
        b = next;
        CK(cmx_rcu_read(&r, fresh, &root) == CMX_OK);
        CK(cmx_rcu_leave(&r, active) == CMX_OK);
        CK(!cmx_rcu_quiescent(&r));
    }
    CK(ag_read_end(&s, b) == AG_OK);
    CK(cmx_rcu_leave(&r, 0) == CMX_OK && cmx_rcu_quiescent(&r));
    puts("CE_PASS:arrivals preexisting_old_reclaimed=1 coarse_waits=1000 "
         "repeatable_nonzero_reader_cycle");
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "registration"))
        registration();
    else if (!strcmp(argv[1], "threads"))
        threads();
    else if (!strcmp(argv[1], "retention"))
        retention();
    else if (!strcmp(argv[1], "arrivals"))
        arrivals();
    else
        return 2;
    return 0;
}
