#define _GNU_SOURCE
#include "timer.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "TIMER-RACE-CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                                   \
        }                                                              \
    } while (0)

struct rd_shared
{
    pthread_mutex_t lock;
    pthread_cond_t changed;
    pthread_barrier_t start;
    struct td_timer timer;
    unsigned token;
    int entered, release, started, calls;
};

static void *callback(void *arg)
{
    struct rd_shared *s = arg;
    struct td_dispatch d;
    int b = pthread_barrier_wait(&s->start);
    CHECK(b == 0 || b == PTHREAD_BARRIER_SERIAL_THREAD);
    CHECK(!pthread_mutex_lock(&s->lock));
    s->started = td_begin(&s->timer, s->token, 1, &d);
    CHECK(s->started >= 0);
    s->entered = 1;
    CHECK(!pthread_cond_broadcast(&s->changed));
    if (s->started)
    {
        /* The callback retains its context while parked. It does not hold lock. */
        while (!s->release)
        {
            CHECK(!pthread_cond_wait(&s->changed, &s->lock));
        }
        ++s->calls;
        CHECK(td_finish(&s->timer, s->token) == 1);
    }
    CHECK(!pthread_mutex_unlock(&s->lock));
    return NULL;
}

static int one(int force)
{
    struct rd_shared s = {0};
    pthread_t thread;
    unsigned replacement = 99;
    CHECK(!pthread_mutex_init(&s.lock, NULL) &&
          !pthread_cond_init(&s.changed, NULL));
    CHECK(!pthread_barrier_init(&s.start, NULL, 2));
    CHECK(td_init(&s.timer, 4) == 0 &&
          td_arm(&s.timer, 1, 1, 0, &s.token) == 1);
    if (force == 2)
    {
        CHECK(td_cancel(&s.timer) == 1);
    }
    CHECK(!pthread_create(&thread, NULL, callback, &s));
    int b = pthread_barrier_wait(&s.start);
    CHECK(b == 0 || b == PTHREAD_BARRIER_SERIAL_THREAD);
    CHECK(!pthread_mutex_lock(&s.lock));
    if (force == 1)
    {
        while (!s.entered)
        {
            CHECK(!pthread_cond_wait(&s.changed, &s.lock));
        }
    }
    int result = td_cancel(&s.timer);
    CHECK(result == (s.started ? 2 : force == 2 ? 0 : 1));
    if (s.started)
    {
        CHECK(td_cancel(&s.timer) == 2 && s.timer.state == TD_CANCELLING);
        CHECK(td_arm(&s.timer, 9, 0, 0, &replacement) == 0 &&
              replacement == 99);
        CHECK(s.calls ==
              0); /* Cancellation did not end this borrowed context. */
    }
    s.release = 1;
    CHECK(!pthread_cond_broadcast(&s.changed));
    CHECK(!pthread_mutex_unlock(&s.lock));
    CHECK(!pthread_join(thread, NULL));
    CHECK(s.timer.state == TD_IDLE && s.calls == s.started &&
          td_cancel(&s.timer) == 0);
    CHECK(!pthread_barrier_destroy(&s.start));
    CHECK(!pthread_cond_destroy(&s.changed) && !pthread_mutex_destroy(&s.lock));
    return s.started;
}

int main(void)
{
    CHECK(one(1) == 1 && one(2) == 0);
    unsigned started = 0;
    for (unsigned i = 0; i < 200; ++i)
    {
        started += (unsigned)one(0);
    }
    printf(
        "TIMER-RACE:forced_running=1;forced_cancelled=1;races=200;started=%u;"
        "prevented=%u;\n",
        started, 200 - started);
}
