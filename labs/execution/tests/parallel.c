#include "cmx.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#define ITEMS 20000u
static cmx_hp_queue queue;
static atomic_uint produced, consumed, errors;
static atomic_uint seen[2 * ITEMS];

static void *worker(void *arg)
{
    unsigned p = (unsigned)(uintptr_t)arg;
    if (p < 2)
    {
        for (unsigned i = 0; i < ITEMS; i++)
        {
            uint32_t v = p * ITEMS + i;
            while (cmx_hp_push(&queue, p, v) == CMX_FULL)
            {
                cmx_hp_collect(&queue);
                sched_yield();
            }
            atomic_fetch_add(&produced, 1);
        }
    }
    else
    {
        for (;;)
        {
            uint32_t v;
            enum cmx_result r = cmx_hp_pop(&queue, p, &v);
            if (r == CMX_OK)
            {
                if (v >= 2 * ITEMS)
                    atomic_fetch_add(&errors, 1);
                else if (atomic_fetch_add(&seen[v], 1))
                    atomic_fetch_add(&errors, 1);
                atomic_fetch_add(&consumed, 1);
                cmx_hp_collect(&queue);
            }
            else if (atomic_load(&produced) == 2 * ITEMS &&
                     atomic_load(&consumed) == 2 * ITEMS)
                break;
            else
                sched_yield();
        }
    }
    return NULL;
}

int main(void)
{
    cmx_hp_init(&queue);
    if (!atomic_is_lock_free(&queue.head))
    {
        fputs("UNAVAILABLE native lock-free unsigned\n", stderr);
        return 77;
    }
    pthread_t threads[4];
    for (unsigned i = 0; i < 4; i++)
        if (pthread_create(&threads[i], NULL, worker, (void *)(uintptr_t)i))
            return 2;
    for (unsigned i = 0; i < 4; i++)
        if (pthread_join(threads[i], NULL))
            return 2;
    if (atomic_load(&errors) || atomic_load(&consumed) != 2 * ITEMS)
        return 1;
    for (unsigned i = 0; i < 2 * ITEMS; i++)
        if (atomic_load(&seen[i]) != 1)
            return 1;
    puts(
        "PASS actual four-pthread MPMC: 40000 unique transfers; finite stress, "
        "not proof");
    return 0;
}
