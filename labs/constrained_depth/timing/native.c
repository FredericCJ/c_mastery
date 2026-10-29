#define _POSIX_C_SOURCE 200809L
#include "timing.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
static struct ct_clock clock_value;
static pthread_barrier_t gate;
static _Atomic unsigned failed;

static void *writer(void *unused)
{
    (void)unused;
    pthread_barrier_wait(&gate);
    for (unsigned i = 1; i <= 10000; i++)
    {
        struct ct_stamp s = {1, i / 256, 250000, i % 256, 8};
        if (ct_publish(&clock_value, s) != CT_OK)
            atomic_store(&failed, 1);
    }
    return NULL;
}

struct cr_reader
{
    unsigned reads, busy;
    uint64_t last;
};

static void *reader(void *context)
{
    struct cr_reader *r = context;
    pthread_barrier_wait(&gate);
    for (unsigned i = 0; i < 20000; i++)
    {
        struct ct_stamp s = {99, 99, 99, 99, 99};
        int result = ct_snapshot(&clock_value, 1, NULL, NULL, &s);
        if (result == CT_BUSY)
        {
            r->busy++;
            if (s.boot != 99)
                atomic_store(&failed, 1);
        }
        else if (result == CT_OK)
        {
            uint64_t count = s.epoch * 256 + s.low;
            if (s.boot != 1 || s.bits != 8 || s.quantum_ns != 250000 ||
                s.low >= 256 || count > 10000 || count < r->last)
                atomic_store(&failed, 1);
            r->last = count;
            r->reads++;
        }
        else
            atomic_store(&failed, 1);
    }
    return NULL;
}

int main(void)
{
    struct ct_stamp initial = {1, 0, 250000, 0, 8};
    if (ct_clock_init(&clock_value, initial) != CT_OK ||
        pthread_barrier_init(&gate, NULL, 3))
        return 2;
    pthread_t threads[3];
    struct cr_reader r[2] = {{0}, {0}};
    if (pthread_create(&threads[0], NULL, writer, NULL) ||
        pthread_create(&threads[1], NULL, reader, &r[0]) ||
        pthread_create(&threads[2], NULL, reader, &r[1]))
        return 2;
    for (unsigned i = 0; i < 3; i++)
        if (pthread_join(threads[i], NULL))
            return 2;
    if (pthread_barrier_destroy(&gate))
        return 2;
    struct ct_stamp final;
    if (ct_snapshot(&clock_value, 1, NULL, NULL, &final) != CT_OK ||
        final.epoch * 256 + final.low != 10000 || atomic_load(&failed) ||
        r[0].reads + r[1].reads + r[0].busy + r[1].busy != 40000)
    {
        puts("CT_NATIVE_CHECK:inconsistent-or-regressing-snapshot");
        return 1;
    }
    printf(
        "CT_NATIVE_PASS:published=10000 reads=%u busy=%u sequence=%llu final=10000 clock-bytes=%zu stamp-bytes=%zu\n",
        r[0].reads + r[1].reads, r[0].busy + r[1].busy,
        (unsigned long long)atomic_load(&clock_value.sequence),
        sizeof(struct ct_clock), sizeof(struct ct_stamp));
    return 0;
}
