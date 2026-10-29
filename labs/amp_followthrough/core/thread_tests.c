#include "amp.h"
#include <pthread.h>
#include <stdio.h>
#include <sched.h>
static struct af_ring q;
static _Atomic unsigned failed;

enum
{
    COUNT = 100000
};

static void *producer(void *unused)
{
    (void)unused;
    for (uint32_t seq = 1; seq <= COUNT;)
    {
        struct af_message v[4];
        unsigned n = COUNT - seq + 1 < 4 ? COUNT - seq + 1 : 4;
        for (unsigned i = 0; i < n; i++)
        {
            v[i] = (struct af_message){
                .epoch = 2, .sequence = seq + i, .length = (seq + i) % 33};
            for (unsigned k = 0; k < v[i].length; k++)
                v[i].bytes[k] = (unsigned char)(seq + i + k);
        }
        int r = af_batch(&q, 2, v, n);
        if (r == AF_OK)
            seq += n;
        else if (r != AF_FULL)
        {
            atomic_store(&failed, 1);
            return 0;
        }
        else
            sched_yield();
    }
    return 0;
}

static void *consumer(void *unused)
{
    (void)unused;
    for (uint32_t want = 1; want <= COUNT;)
    {
        unsigned char bytes[32];
        size_t len;
        uint32_t seq;
        int r = af_receive(&q, 2, bytes, 32, &seq, &len);
        if (r == AF_EMPTY)
        {
            sched_yield();
            continue;
        }
        if (r != AF_OK || seq != want || len != want % 33)
        {
            atomic_store(&failed, 1);
            return 0;
        }
        for (unsigned k = 0; k < len; k++)
            if (bytes[k] != (unsigned char)(want + k))
                atomic_store(&failed, 1);
        want++;
    }
    return 0;
}

int main(void)
{
    pthread_t a, b;
    af_ring_init(&q, UINT32_MAX - 7);
    if (pthread_create(&a, 0, producer, 0) ||
        pthread_create(&b, 0, consumer, 0))
        return 2;
    if (pthread_join(a, 0) || pthread_join(b, 0))
        return 3;
    if (atomic_load(&failed) || af_queued(&q))
        return 1;
    puts("AF_PASS:threaded 100000 messages batch4 variable-length wrap");
    return 0;
}
