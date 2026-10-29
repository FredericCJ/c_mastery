#include "interrupts.h"
#include <pthread.h>
#include <stdio.h>
#include <sched.h>

enum
{
    OWNERS = 4,
    BATCHES = 20000
};

struct sa_owner
{
    struct si_box box;
    uint64_t sum;
    unsigned calls, failed;
};
static struct sa_owner owners[OWNERS];

static void add(void *context, uint32_t item)
{
    struct sa_owner *owner = context;
    if (atomic_load_explicit(&owner->box.acknowledged, memory_order_acquire) ==
        atomic_load_explicit(&owner->box.published, memory_order_relaxed))
    {
        owner->failed = 1;
    }
    owner->sum += item;
    owner->calls++;
}

static void *consume(void *context)
{
    struct sa_owner *owner = context;
    for (uint64_t generation = 1; generation <= BATCHES; generation++)
    {
        while (atomic_load_explicit(&owner->box.published,
                                    memory_order_acquire) != generation)
        {
            sched_yield();
        }
        if (si_drain(&owner->box, generation, add, owner) != SI_OK)
        {
            owner->failed = 1;
        }
    }
    return NULL;
}

int main(void)
{
    pthread_t threads[OWNERS];
    for (unsigned i = 0; i < OWNERS; i++)
    {
        si_init(&owners[i].box);
        if (pthread_create(&threads[i], NULL, consume, &owners[i]))
        {
            return 2;
        }
    }
    for (unsigned generation = 1; generation <= BATCHES; generation++)
    {
        for (unsigned i = 0; i < OWNERS; i++)
        {
            uint32_t data[4] = {generation, generation + 1, generation + 2,
                                generation + 3};
            uint64_t ticket = 0;
            while (generation > 1 &&
                   si_status(&owners[i].box, generation - 1) != SI_OK)
            {
                sched_yield();
            }
            if (si_publish(&owners[i].box, data, 4, &ticket) != SI_OK ||
                ticket != generation)
            {
                return 3;
            }
        }
    }
    for (unsigned i = 0; i < OWNERS; i++)
    {
        if (pthread_join(threads[i], NULL))
        {
            return 4;
        }
        uint64_t expected =
            4 * (uint64_t)BATCHES * (BATCHES + 1) / 2 + 6 * BATCHES;
        if (owners[i].failed || owners[i].calls != 4 * BATCHES ||
            owners[i].sum != expected ||
            si_status(&owners[i].box, BATCHES) != SI_OK)
        {
            return 5;
        }
    }
    puts("SI_NATIVE_PASS:4-exclusive-consumers-80000-batches-320000-items");
    return 0;
}
