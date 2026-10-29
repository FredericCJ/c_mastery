#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "logger.h"
#define CHECK(x)                                                          \
    do                                                                    \
    {                                                                     \
        if (!(x))                                                         \
        {                                                                 \
            fprintf(stderr, "EFFECTS_LOCK_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(1);                                                      \
        }                                                                 \
    } while (0)
static pthread_mutex_t held = PTHREAD_MUTEX_INITIALIZER,
                       gate = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t changed = PTHREAD_COND_INITIALIZER;
static int ready, finished;
static double waited;

static double now(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static void *waiter(void *ignored)
{
    (void)ignored;
    CHECK(pthread_mutex_lock(&gate) == 0);
    ready = 1;
    CHECK(pthread_cond_signal(&changed) == 0);
    CHECK(pthread_mutex_unlock(&gate) == 0);
    double start = now();
    CHECK(pthread_mutex_lock(&held) == 0);
    waited = now() - start;
    finished = 1;
    CHECK(pthread_mutex_unlock(&held) == 0);
    return NULL;
}
static struct ef_log log_buffer = {ATOMIC_FLAG_INIT, {0}, 0};

static void *logger(void *v)
{
    unsigned char marker = (unsigned char)(size_t)v;
    for (unsigned i = 0; i < 10000; ++i)
    {
        int result = ef_log(&log_buffer, &marker, 1);
        CHECK(result == 0 || result == 1);
    }
    return NULL;
}

int main(void)
{
    CHECK(pthread_mutex_lock(&held) == 0);
    pthread_t worker;
    CHECK(pthread_create(&worker, NULL, waiter, NULL) == 0);
    CHECK(pthread_mutex_lock(&gate) == 0);
    while (!ready)
        CHECK(pthread_cond_wait(&changed, &gate) == 0);
    CHECK(pthread_mutex_unlock(&gate) == 0);
    /* Trylock is an observed busy result, not a recursive blocking test. */
    CHECK(pthread_mutex_trylock(&held) == EBUSY);
    struct timespec pause = {0, 20000000};
    while (nanosleep(&pause, &pause) && errno == EINTR)
    {
    }
    CHECK(pthread_mutex_unlock(&held) == 0 && pthread_join(worker, NULL) == 0 &&
          finished);
    double start = now();
    CHECK(pthread_mutex_lock(&held) == 0 && pthread_mutex_unlock(&held) == 0);
    double uncontended = now() - start;
    /* Deterministic wait-for graph: owner A waits B, B waits C, C waits A.
     * No process is deliberately deadlocked and no scheduling proof is inferred. */
    const unsigned next[3] = {1, 2, 0};
    unsigned vertex = 0;
    for (unsigned i = 0; i < 3; ++i)
        vertex = next[vertex];
    CHECK(vertex == 0);
    pthread_t a, b;
    CHECK(pthread_create(&a, NULL, logger, (void *)(size_t)1) == 0 &&
          pthread_create(&b, NULL, logger, (void *)(size_t)2) == 0);
    CHECK(pthread_join(a, NULL) == 0 && pthread_join(b, NULL) == 0);
    CHECK(log_buffer.length == 1 &&
          (log_buffer.bytes[0] == 1 || log_buffer.bytes[0] == 2));
    printf(
        "{\"contended_wait_seconds\":%.9f,\"uncontended_seconds\":%.9f,\"trylock_busy\":true,\"modeled_cycle_length\":3,\"logging_attempts\":20000}\n",
        waited, uncontended);
    return 0;
}
