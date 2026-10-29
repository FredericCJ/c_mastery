#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

typedef struct
{
    unsigned data[8], head, count, closed;
    pthread_mutex_t lock;
    pthread_cond_t ready, space;
} queue;

static queue q = {.lock = PTHREAD_MUTEX_INITIALIZER,
                  .ready = PTHREAD_COND_INITIALIZER,
                  .space = PTHREAD_COND_INITIALIZER};
static unsigned received, sum;

static void *producer(void *unused)
{
    (void)unused;
    for (unsigned i = 0; i < 256; ++i)
    {
        assert(!pthread_mutex_lock(&q.lock));
        while (q.count == 8)
            assert(!pthread_cond_wait(&q.space, &q.lock));
        q.data[(q.head + q.count) % 8] = i;
        ++q.count;
        assert(!pthread_cond_signal(&q.ready));
        assert(!pthread_mutex_unlock(&q.lock));
    }
    assert(!pthread_mutex_lock(&q.lock));
    q.closed = 1;
    assert(!pthread_cond_broadcast(&q.ready));
    assert(!pthread_mutex_unlock(&q.lock));
    return NULL;
}

static void *consumer(void *unused)
{
    (void)unused;
    for (;;)
    {
        assert(!pthread_mutex_lock(&q.lock));
        while (!q.count && !q.closed)
            assert(!pthread_cond_wait(&q.ready, &q.lock));
        if (!q.count)
        {
            assert(q.closed);
            assert(!pthread_mutex_unlock(&q.lock));
            break;
        }
        unsigned item = q.data[q.head];
        q.head = (q.head + 1) % 8;
        --q.count;
        assert(!pthread_cond_signal(&q.space));
        assert(!pthread_mutex_unlock(&q.lock));
        assert(item == received);
        ++received;
        sum += item;
    }
    return NULL;
}

static double seconds(struct timespec a, struct timespec b)
{
    return (double)(b.tv_sec - a.tv_sec) +
           (double)(b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void)
{
    struct timespec w0, w1, c0, c1;
    pthread_t p, c;
    assert(!clock_gettime(CLOCK_MONOTONIC, &w0));
    assert(!clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &c0));
    assert(!pthread_create(&c, NULL, consumer, NULL));
    assert(!pthread_create(&p, NULL, producer, NULL));
    assert(!pthread_join(p, NULL));
    assert(!pthread_join(c, NULL));
    assert(!clock_gettime(CLOCK_MONOTONIC, &w1));
    assert(!clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &c1));
    assert(received == 256 && sum == 32640 && q.count == 0);
    printf(
        "platform=Linux-pthreads count=%u checksum=%u capacity=8 wall_s=%.9f cpu_s=%.9f\n",
        received, sum, seconds(w0, w1), seconds(c0, c1));
    assert(!pthread_cond_destroy(&q.ready));
    assert(!pthread_cond_destroy(&q.space));
    assert(!pthread_mutex_destroy(&q.lock));
    return 0;
}
