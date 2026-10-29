#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                  \
    do                                                            \
    {                                                             \
        if (!(x))                                                 \
        {                                                         \
            fprintf(stderr, "DX_SCHEDULE:%d:%s\n", __LINE__, #x); \
            exit(1);                                              \
        }                                                         \
    } while (0)
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER,
                       a = PTHREAD_MUTEX_INITIALIZER,
                       b = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static pthread_barrier_t checked, signaled;
static unsigned predicate;
static int outcome[2];
static atomic_uint entering;

static void gate(pthread_barrier_t *p)
{
    int e = pthread_barrier_wait(p);
    CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
}

static struct timespec deadline(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_REALTIME, &t) == 0);
    t.tv_nsec += 200000000;
    if (t.tv_nsec >= 1000000000)
    {
        t.tv_sec++;
        t.tv_nsec -= 1000000000;
    }
    return t;
}

__attribute__((noinline)) void dx_thread_marker(void)
{
    __asm__ volatile("" ::: "memory");
}

__attribute__((noinline)) void dx_wait_snapshot(void)
{
    __asm__ volatile("" ::: "memory");
}

static void observe_waits(unsigned count)
{
    for (unsigned n = 0; n < 100 && atomic_load(&entering) < count; n++)
    {
        struct timespec pause = {0, 1000000};
        nanosleep(&pause, NULL);
    }
    CHECK(atomic_load(&entering) == count);
    struct timespec pause = {0, 20000000};
    nanosleep(&pause, NULL);
    dx_wait_snapshot();
}

static void *consumer(void *unused)
{
    (void)unused;
    CHECK(pthread_mutex_lock(&mutex) == 0);
    CHECK(predicate == 0);
    CHECK(pthread_mutex_unlock(&mutex) == 0);
    gate(&checked);
    gate(&signaled);
    CHECK(pthread_mutex_lock(&mutex) == 0);
    dx_thread_marker();
    struct timespec end =
        deadline(); /* Deliberate lost notification: predicate is not rechecked. No data race. */
    atomic_fetch_add(&entering, 1);
    outcome[0] = pthread_cond_timedwait(&cv, &mutex, &end);
    CHECK(pthread_mutex_unlock(&mutex) == 0);
    return NULL;
}

static void *producer(void *unused)
{
    (void)unused;
    gate(&checked);
    CHECK(pthread_mutex_lock(&mutex) == 0);
    predicate = 1;
    CHECK(pthread_cond_signal(&cv) == 0);
    CHECK(pthread_mutex_unlock(&mutex) == 0);
    gate(&signaled);
    return NULL;
}

static void *cycle(void *arg)
{
    unsigned index = *(unsigned *)arg;
    pthread_mutex_t *first = index ? &b : &a, *second = index ? &a : &b;
    CHECK(pthread_mutex_lock(first) == 0);
    gate(&checked);
    dx_thread_marker();
    struct timespec end = deadline();
    atomic_fetch_add(&entering, 1);
    outcome[index] = pthread_mutex_timedlock(second, &end);
    if (!outcome[index])
        CHECK(pthread_mutex_unlock(second) == 0);
    else
        CHECK(outcome[index] == ETIMEDOUT);
    CHECK(pthread_mutex_unlock(first) == 0);
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t ts[2];
    CHECK(pthread_barrier_init(&checked, NULL, 2) == 0);
    CHECK(pthread_barrier_init(&signaled, NULL, 2) == 0);
    if (argc > 1 && !strcmp(argv[1], "cycle"))
    {
        unsigned ids[] = {0, 1};
        for (unsigned i = 0; i < 2; i++)
            CHECK(pthread_create(&ts[i], NULL, cycle, &ids[i]) == 0);
        observe_waits(2);
        for (unsigned i = 0; i < 2; i++)
            CHECK(pthread_join(ts[i], NULL) == 0);
        CHECK(outcome[0] == ETIMEDOUT || outcome[1] == ETIMEDOUT);
        puts("DX:wait-cycle:forced-opposite-ownership:timeout-breaks-cycle");
    }
    else
    {
        CHECK(pthread_create(&ts[0], NULL, consumer, NULL) == 0 &&
              pthread_create(&ts[1], NULL, producer, NULL) == 0);
        observe_waits(1);
        for (unsigned i = 0; i < 2; i++)
            CHECK(pthread_join(ts[i], NULL) == 0);
        CHECK(outcome[0] == ETIMEDOUT && predicate == 1);
        puts("DX:lost-wake:false-check:set-signal:wait-timeout:predicate1");
    }
    CHECK(pthread_barrier_destroy(&checked) == 0 &&
          pthread_barrier_destroy(&signaled) == 0);
    CHECK(pthread_cond_destroy(&cv) == 0);
    CHECK(pthread_mutex_destroy(&mutex) == 0 &&
          pthread_mutex_destroy(&a) == 0 && pthread_mutex_destroy(&b) == 0);
    return 0;
}
