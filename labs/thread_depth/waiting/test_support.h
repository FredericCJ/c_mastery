#ifndef TW_TEST_SUPPORT_H
#define TW_TEST_SUPPORT_H
#include "waiting.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "TW_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            exit(41);                                                        \
        }                                                                    \
    } while (0)

static inline void gate(pthread_barrier_t *b)
{
    int e = pthread_barrier_wait(b);
    CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
}

static inline struct timespec later(long milliseconds)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    t.tv_sec += milliseconds / 1000;
    t.tv_nsec += (milliseconds % 1000) * 1000000;
    if (t.tv_nsec >= 1000000000)
    {
        t.tv_sec++;
        t.tv_nsec -= 1000000000;
    }
    return t;
}

void test_bank(void);
void test_order(void);
void test_lifecycle(void);
void test_queue(void);
void test_wake(void);
void test_deadline(void);
void test_barrier(void);
void test_fairness(void);
void test_permit(void);
#endif
