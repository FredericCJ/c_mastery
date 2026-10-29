#include "supervision.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
static struct cs_world world;
static atomic_uint failed, done;

static void *publisher(void *arg)
{
    unsigned id = *(unsigned *)arg;
    for (unsigned n = 0; n < 10000; ++n)
    {
        enum cs_status s;
        do
        {
            s = cs_progress(&world, id);
            if (s == CS_BUSY)
            {
                sched_yield();
            }
        } while (s == CS_BUSY);
        if (s != CS_OK)
        {
            atomic_store(&failed, 1);
        }
    }
    atomic_fetch_add(&done, 1);
    return 0;
}

int main(void)
{
    pthread_t thread[2];
    unsigned ids[2] = {0, 1};
    if (cs_init(&world, 2, 1) != CS_OK)
    {
        return 1;
    }
    atomic_init(&failed, 0);
    atomic_init(&done, 0);
    for (unsigned i = 0; i < 2; ++i)
    {
        if (pthread_create(&thread[i], 0, publisher, &ids[i]))
        {
            return 1;
        }
    }
    unsigned feeds = 0;
    while (atomic_load(&done) != 2)
    {
        enum cs_status s = cs_window(&world);
        if (s == CS_OK)
        {
            ++feeds;
        }
        else if (s != CS_WAIT && s != CS_BUSY)
        {
            atomic_store(&failed, 1);
        }
        sched_yield();
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        if (pthread_join(thread[i], 0))
        {
            return 1;
        }
    }
    enum cs_status last = cs_window(&world);
    if (last == CS_OK)
    {
        ++feeds;
    }
    if (atomic_load(&failed) || !feeds ||
        atomic_load(&world.epoch[0]) != 10000 ||
        atomic_load(&world.epoch[1]) != 10000 || world.last[0] > 10000 ||
        world.last[1] > 10000)
    {
        return 1;
    }
    puts(
        "CS_NATIVE_PASS:publishers=2 commits=20000 coherent-supervisor=joined");
    return 0;
}
