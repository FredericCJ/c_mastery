#define _POSIX_C_SOURCE 200809L
#include "history.h"
#include "reclamation.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "AP_HISTORY_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(41);                                                   \
        }                                                               \
    } while (0)

struct ag_work
{
    struct ap_queue *q;
    struct ap_lease lease;
    struct ah_operation *op;
    atomic_uint *clock, *ready;
    unsigned actor;
};

static void *work(void *ptr)
{
    struct ag_work *w = ptr;
    atomic_fetch_add(w->ready, 1);
    while (atomic_load(w->ready) < 4)
    {
        sched_yield();
    }
    struct ah_operation *op = w->op;
    op->participant = w->actor;
    op->operation = w->actor < 2 ? AH_INSERT : AH_REMOVE;
    op->invoke = atomic_fetch_add(w->clock, 1);
    struct ap_value v = {w->actor + 1, ~(w->actor + 1)};
    if (w->actor < 2)
    {
        CHECK(ap_push(w->q, w->lease, v) == AP_OK);
        op->value = v.sequence;
        op->result = AH_VALUE;
    }
    else
    {
        int r = ap_pop(w->q, w->lease, &v);
        CHECK(r == AP_OK || r == AP_RETRY);
        op->result = r == AP_OK ? AH_VALUE : AH_EMPTY;
        op->value = r == AP_OK ? v.sequence : 0;
        if (r == AP_OK)
        {
            CHECK(v.inverse == ~v.sequence);
        }
    }
    op->respond = atomic_fetch_add(w->clock, 1);
    CHECK(ap_leave(w->q, w->lease) == AP_OK);
    return NULL;
}

static void replay(const struct ah_operation *ops, const struct ah_witness *w)
{
    unsigned fifo[4], n = 0, used = 0;
    CHECK(w->length == 4 && w->completed_pending == 0);
    for (unsigned i = 0; i < 4; i++)
    {
        unsigned j = w->order[i];
        CHECK(j < 4 && !(used & (1u << j)));
        for (unsigned k = 0; k < 4; k++)
        {
            if (ops[k].respond < ops[j].invoke)
            {
                CHECK(used & (1u << k));
            }
        }
        used |= 1u << j;
        CHECK(w->result[j] == ops[j].result && w->value[j] == ops[j].value);
        if (ops[j].operation == AH_INSERT)
        {
            CHECK(n < 4);
            fifo[n++] = ops[j].value;
        }
        else if (ops[j].result == AH_EMPTY)
        {
            CHECK(n == 0);
        }
        else
        {
            CHECK(n && fifo[0] == ops[j].value);
            for (unsigned k = 1; k < n; k++)
            {
                fifo[k - 1] = fifo[k];
            }
            n--;
        }
    }
}

int main(void)
{
    for (unsigned run = 0; run < 64; run++)
    {
        struct ap_queue q;
        CHECK(ap_init(&q, 4, UINT_MAX >> 2, NULL, NULL) == AP_OK);
        struct ah_operation ops[4];
        struct ag_work args[4];
        pthread_t threads[4];
        atomic_uint clock, ready;
        atomic_init(&clock, 0);
        atomic_init(&ready, 0);
        for (unsigned i = 0; i < 4; i++)
        {
            struct ap_lease lease;
            CHECK(ap_register(&q, &lease) == AP_OK);
            args[i] = (struct ag_work){&q, lease, &ops[i], &clock, &ready, i};
        }
        for (unsigned i = 0; i < 4; i++)
        {
            CHECK(!pthread_create(&threads[i], NULL, work, &args[i]));
        }
        for (unsigned i = 0; i < 4; i++)
        {
            CHECK(!pthread_join(threads[i], NULL));
        }
        struct ah_witness w;
        CHECK(ah_history(ops, 4, 4, AH_FIFO, AH_COMPLETE_ONLY, &w) == AH_VALID);
        replay(ops, &w);
        printf("TRACE %u", run);
        for (unsigned i = 0; i < 4; i++)
        {
            printf(" %u,%u,%u,%u,%u,%u", ops[i].invoke, ops[i].respond,
                   ops[i].participant, ops[i].operation, ops[i].value,
                   ops[i].result);
        }
        puts("");
    }
    struct ah_operation bad[] = {{0, 1, 0, AH_INSERT, 11, AH_VALUE},
                                 {2, 3, 1, AH_INSERT, 22, AH_VALUE},
                                 {4, 5, 2, AH_REMOVE, 22, AH_VALUE},
                                 {6, 7, 3, AH_REMOVE, 11, AH_VALUE}};
    struct ah_witness w;
    CHECK(ah_history(bad, 4, 4, AH_FIFO, AH_COMPLETE_ONLY, &w) == AH_NONLINEAR);
    puts("OBS histories actual_calls=256 histories=64 real_time_fifo=validated "
         "witness_replay=64 reversed_fifo=rejected");
    return 0;
}
