#include "follow.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef COUNT
#define COUNT 10000
#endif
static struct b3_mailbox old;
static struct bf_box box;
static atomic_uint failed;

static void problem(void)
{
    atomic_store(&failed, 1);
}

static int retry(unsigned *n)
{
    (*n)++;
    if (*n > 100000000u || atomic_load(&failed))
    {
        problem();
        return 0;
    }
    if (!(*n % 128))
    {
        sched_yield();
    }
    return 1;
}

static void *old_producer(void *v)
{
    int i;
    (void)v;
    for (i = 1; i <= COUNT; i++)
    {
        unsigned tries = 0;
        int rc;
        while ((rc = b3_publish(&old, i)) == B3_WAIT)
        {
            if (!retry(&tries))
            {
                return NULL;
            }
        }
        if (rc != B3_OK)
        {
            problem();
            return NULL;
        }
    }
    return NULL;
}

static void *old_consumer(void *v)
{
    int i;
    (void)v;
    for (i = 1; i <= COUNT; i++)
    {
        unsigned tries = 0;
        int rc, value = 0;
        while ((rc = b3_take(&old, &value)) == B3_WAIT)
        {
            if (!retry(&tries))
            {
                return NULL;
            }
        }
        if (rc != B3_OK || value != i)
        {
            problem();
            return NULL;
        }
    }
    return NULL;
}

static void *producer(void *v)
{
    int id = *(int *)v, i;
    for (i = 1; i <= COUNT; i++)
    {
        unsigned tries = 0;
        int rc;
        while ((rc = bf_put(&box, id * COUNT + i)) == B3_WAIT)
        {
            if (!retry(&tries))
            {
                return NULL;
            }
        }
        if (rc != B3_OK)
        {
            problem();
            return NULL;
        }
    }
    return NULL;
}

static void *consumer(void *v)
{
    int last[2] = {0}, i;
    (void)v;
    for (i = 0; i < 2 * COUNT; i++)
    {
        unsigned tries = 0;
        int rc, value = 0, id, sequence;
        while ((rc = bf_get(&box, &value)) == B3_WAIT)
        {
            if (!retry(&tries))
            {
                return NULL;
            }
        }
        if (rc != B3_OK || value < 1 || value > 2 * COUNT)
        {
            problem();
            return NULL;
        }
        id = (value - 1) / COUNT;
        sequence = (value - 1) % COUNT + 1;
        if (sequence != last[id] + 1)
        {
            problem();
            return NULL;
        }
        last[id] = sequence;
    }
    if (last[0] != COUNT || last[1] != COUNT)
    {
        problem();
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b, c;
    int ids[2] = {0, 1};
    atomic_init(&failed, 0);
    b3_mailbox_init(&old);
    bf_box_init(&box);
    printf(
        "ATOMIC_SUPPORT size=%zu align=%zu spsc_lock_free=%d claim_lock_free=%d\n",
        sizeof(atomic_uint), _Alignof(atomic_uint),
        atomic_is_lock_free(&old.ready), atomic_is_lock_free(&box.state));
    if (pthread_create(&a, NULL, old_producer, NULL) ||
        pthread_create(&b, NULL, old_consumer, NULL))
    {
        return 2;
    }
    if (pthread_join(a, NULL) || pthread_join(b, NULL) || atomic_load(&failed))
    {
        return 1;
    }
    if (pthread_create(&a, NULL, producer, &ids[0]) ||
        pthread_create(&b, NULL, producer, &ids[1]) ||
        pthread_create(&c, NULL, consumer, NULL))
    {
        return 2;
    }
    if (pthread_join(a, NULL) || pthread_join(b, NULL) ||
        pthread_join(c, NULL) || atomic_load(&failed))
    {
        return 1;
    }
    printf("THREAD_PASS spsc=%d claim=%d producer_order=2\n", COUNT, 2 * COUNT);
    return 0;
}
