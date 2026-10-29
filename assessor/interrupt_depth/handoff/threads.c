#include "handoff.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

/* Two producers NEVER access one SPSC queue. A single peripheral/deferred owner
 * drains two queues; FIFO holds per source, not a global event-time order. */
enum
{
    COUNT = 2000
};
static struct xi_queue queues[2];
static atomic_uint failed;

static void *produce(void *arg)
{
    unsigned owner = (unsigned)(uintptr_t)arg;
    for (unsigned i = 0; i < COUNT; ++i)
    {
        struct xi_record r = {.sequence = i,
                              .timestamp = owner,
                              .original_length = 8,
                              .copied = 8};
        for (unsigned j = 0; j < 8; ++j)
        {
            r.bytes[j] = (unsigned char)(i + j + owner);
        }
        int rc;
        while ((rc = xi_push(&queues[owner], &r)) == XI_FULL)
        {
            if (atomic_load(&failed))
            {
                return NULL;
            }
            sched_yield();
        }
        if (rc)
        {
            atomic_store(&failed, 1);
            return NULL;
        }
    }
    return NULL;
}

int main(void)
{
    pthread_t threads[2];
    unsigned seen[2] = {0};
    atomic_init(&failed, 0);
    for (unsigned i = 0; i < 2; ++i)
    {
        if (xi_init(&queues[i], 4) ||
            pthread_create(&threads[i], NULL, produce, (void *)(uintptr_t)i))
        {
            return 2;
        }
    }
    while (seen[0] + seen[1] < 2 * COUNT && !atomic_load(&failed))
    {
        for (unsigned i = 0; i < 2; ++i)
        {
            struct xi_record r;
            int rc = xi_pop(&queues[i], &r);
            if (rc == XI_EMPTY)
            {
                sched_yield();
                continue;
            }
            if (rc || r.sequence != seen[i] || r.timestamp != i ||
                r.copied != 8)
            {
                atomic_store(&failed, 1);
                break;
            }
            for (unsigned j = 0; j < 8; ++j)
            {
                if (r.bytes[j] != (unsigned char)(seen[i] + j + i))
                {
                    atomic_store(&failed, 1);
                }
            }
            ++seen[i];
        }
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        if (pthread_join(threads[i], NULL))
        {
            return 2;
        }
    }
    if (atomic_load(&failed) || seen[0] != COUNT || seen[1] != COUNT)
    {
        return 1;
    }
    puts("XI_THREADS_PASS owners=2 records=4000");
    return 0;
}
