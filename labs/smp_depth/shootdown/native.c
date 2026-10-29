#include "shootdown.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
static struct st_world world;
static atomic_uint ready, stop, bad, entries;
static atomic_uint_fast64_t observed[4];

static int publish(void *ctx, struct st_request r)
{
    (void)ctx;
    (void)r;
    return 1;
}

static int invalidate(void *ctx, unsigned cpu, struct st_request r)
{
    (void)ctx;
    atomic_store_explicit(&observed[cpu], r.generation, memory_order_release);
    return 1;
}

static int notify(void *ctx, struct st_request r, unsigned targets)
{
    (void)ctx;
    (void)r;
    (void)targets;
    return 1;
}

static void *worker(void *arg)
{
    unsigned cpu = *(unsigned *)arg;
    int active = 0;
    if (st_enter(&world, cpu, 0, NULL) != ST_OK)
    {
        atomic_store(&bad, 1);
        return NULL;
    }
    active = 1;
    if (cpu == 3)
        atomic_fetch_add(&entries, 1);
    atomic_fetch_add(&ready, 1);
    while (!atomic_load(&stop))
    {
        struct st_request r;
        int result = st_request(&world, 0, &r);
        if (result == ST_OK && (r.targets & (1u << cpu)))
        {
            result = st_handle(&world, cpu, r);
            if (result != ST_OK && result != ST_STALE &&
                result != ST_INCOMPLETE)
                atomic_store(&bad, 1);
        }
        if (cpu == 3)
        {
            if (active)
            {
                result = st_leave(&world, cpu, NULL);
                if (result == ST_OK)
                    active = 0;
                else if (result != ST_BUSY)
                    atomic_store(&bad, 1);
            }
            else
            {
                result = st_enter(&world, cpu, 0, NULL);
                if (result == ST_OK)
                {
                    active = 1;
                    atomic_fetch_add(&entries, 1);
                }
                else if (result != ST_BUSY)
                    atomic_store(&bad, 1);
            }
        }
        sched_yield();
    }
    if (active && st_leave(&world, cpu, NULL) != ST_OK)
        atomic_store(&bad, 1);
    return NULL;
}

int main(void)
{
    if (st_init(&world, 4, 101, 202,
                (struct st_port){publish, invalidate, notify, NULL}) != ST_OK ||
        st_enter(&world, 0, 0, NULL) != ST_OK)
        return 1;
    unsigned cpus[3] = {1, 2, 3};
    pthread_t threads[3];
    for (unsigned i = 0; i < 3; i++)
        if (pthread_create(&threads[i], NULL, worker, &cpus[i]))
            return 2;
    while (atomic_load(&ready) != 3 && !atomic_load(&bad))
        sched_yield();
    unsigned with_entry = 0, without_entry = 0;
    for (unsigned iteration = 0; iteration < 400; iteration++)
    {
        struct st_request r;
        struct st_permit permit;
        if (st_begin(&world, 0, 0, 4096, 1, ST_PAGE, &r, NULL) != ST_OK ||
            st_handle(&world, 0, r) != ST_OK)
            return 1;
        if (r.targets & 8)
            with_entry++;
        else
            without_entry++;
        for (;;)
        {
            int result = st_finish(&world, r, 0, &permit, NULL);
            if (result == ST_OK)
                break;
            if (result != ST_INCOMPLETE || atomic_load(&bad))
                return 1;
            sched_yield();
        }
        for (unsigned cpu = 0; cpu < 4; cpu++)
            if ((r.targets & (1u << cpu)) &&
                atomic_load_explicit(&observed[cpu], memory_order_acquire) <
                    r.generation)
                return 1;
    }
    atomic_store(&stop, 1);
    for (unsigned i = 0; i < 3; i++)
        if (pthread_join(threads[i], NULL))
            return 2;
    if (atomic_load(&bad) || !atomic_load(&entries) ||
        st_leave(&world, 0, NULL) != ST_OK)
        return 1;
    printf(
        "ST_NATIVE_PASS:membership-handler-publication requests=400 entered_targets=%u absent_targets=%u entries=%u\n",
        with_entry, without_entry, atomic_load(&entries));
    return 0;
}
