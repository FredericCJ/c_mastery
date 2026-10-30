/*
 * Lock observations: one contended mutex, an uncontended lock, a finite
 * wait-for graph and two threads that call the logger. Every thread is joined
 * before main reports.
 */
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

#define NANOSECONDS_PER_SECOND 1e9
#define HOLD_PAUSE_NS 20000000L
#define LOG_ATTEMPTS_PER_THREAD 10000u
#define LOG_THREADS 2u

static pthread_mutex_t held = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gate = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t changed = PTHREAD_COND_INITIALIZER;

/* Guarded by gate: the waiter sets it once it is about to contend for held. */
static int ready;

/* Written by the waiter, read by main only after pthread_join. */
static int finished;
static double waited_seconds;

static double now_seconds(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (double)t.tv_sec + (double)t.tv_nsec / NANOSECONDS_PER_SECOND;
}

static void *waiter(void *ignored)
{
    (void)ignored;
    CHECK(pthread_mutex_lock(&gate) == 0);
    ready = 1;
    CHECK(pthread_cond_signal(&changed) == 0);
    CHECK(pthread_mutex_unlock(&gate) == 0);
    double start = now_seconds();
    CHECK(pthread_mutex_lock(&held) == 0);
    waited_seconds = now_seconds() - start;
    finished = 1;
    CHECK(pthread_mutex_unlock(&held) == 0);
    return NULL;
}

static struct ef_log log_buffer = {ATOMIC_FLAG_INIT, {0}, 0};

/* The thread argument carries a one-byte marker, not a pointer. */
static void *logger(void *marker_value)
{
    unsigned char marker = (unsigned char)(size_t)marker_value;
    for (unsigned i = 0; i < LOG_ATTEMPTS_PER_THREAD; ++i)
    {
        int result = ef_log(&log_buffer, &marker, 1);
        CHECK(result == 0 || result == 1);
    }
    return NULL;
}

/* Holds `held` until the waiter has signalled and a short pause elapsed. */
static void contend_for_held_mutex(void)
{
    CHECK(pthread_mutex_lock(&held) == 0);
    pthread_t worker;
    CHECK(pthread_create(&worker, NULL, waiter, NULL) == 0);
    CHECK(pthread_mutex_lock(&gate) == 0);
    while (!ready)
    {
        CHECK(pthread_cond_wait(&changed, &gate) == 0);
    }
    CHECK(pthread_mutex_unlock(&gate) == 0);
    /* Trylock is an observed busy result, not a recursive blocking test. */
    CHECK(pthread_mutex_trylock(&held) == EBUSY);
    struct timespec pause = {0, HOLD_PAUSE_NS};
    while (nanosleep(&pause, &pause) && errno == EINTR)
    {
    }
    CHECK(pthread_mutex_unlock(&held) == 0 && pthread_join(worker, NULL) == 0 &&
          finished);
}

static double time_uncontended_lock(void)
{
    double start = now_seconds();
    CHECK(pthread_mutex_lock(&held) == 0 && pthread_mutex_unlock(&held) == 0);
    return now_seconds() - start;
}

/*
 * Deterministic wait-for graph: owner A waits for B, B for C, C for A. Walking
 * three edges from A returns to A. No thread is blocked on purpose.
 */
static void walk_wait_for_cycle(void)
{
    const unsigned next[3] = {1, 2, 0};
    unsigned vertex = 0;
    for (unsigned i = 0; i < 3; ++i)
    {
        vertex = next[vertex];
    }
    CHECK(vertex == 0);
}

static void run_logger_threads(void)
{
    pthread_t a;
    pthread_t b;
    CHECK(pthread_create(&a, NULL, logger, (void *)(size_t)1) == 0 &&
          pthread_create(&b, NULL, logger, (void *)(size_t)2) == 0);
    CHECK(pthread_join(a, NULL) == 0 && pthread_join(b, NULL) == 0);
    CHECK(log_buffer.length == 1 &&
          (log_buffer.bytes[0] == 1 || log_buffer.bytes[0] == 2));
}

int main(void)
{
    contend_for_held_mutex();
    double uncontended_seconds = time_uncontended_lock();
    walk_wait_for_cycle();
    run_logger_threads();
    printf(
        "{\"contended_wait_seconds\":%.9f,\"uncontended_seconds\":%.9f,\"trylock_busy\":true,\"modeled_cycle_length\":3,\"logging_attempts\":%u}\n",
        waited_seconds, uncontended_seconds,
        LOG_THREADS * LOG_ATTEMPTS_PER_THREAD);
    return 0;
}
