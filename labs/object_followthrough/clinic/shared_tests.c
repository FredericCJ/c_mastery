/*
 * Test driver for the shared-payload part of the clinic laboratory.
 *
 * Usage: shared_tests [serial|late]; no argument (or any other) runs the
 * parallel case. Markers on success: MC_SHARED_PARALLEL_PASS,
 * MC_SHARED_SERIAL_PASS, MC_SHARED_LATE_PASS.
 *   parallel  two threads add to the payload at once (barrier-aligned start)
 *   serial    the worker finishes before the creator starts adding
 *   late      the creator drops its reference while the worker still holds
 *             the last one and then uses it
 * Exit status: 0 pass; 41 a REQUIRE failed (text on stderr after the marker
 * MC_SHARED_ASSERT:).
 */
#define _POSIX_C_SOURCE 200809L
#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
    ADDS_PER_THREAD = 10000, /* mc_shared_add(1) calls made by each thread */
    LATE_WORKER_AMOUNT = 7,
    THREADS_AT_BARRIER = 2
};

static void *allocation; /* last block handed out, freed on early exit */
static unsigned frees; /* release callbacks seen */
static int fail_allocation;
static int launched; /* nonzero while the worker may still own the payload */

static void *allocate(void *c, size_t n)
{
    (void)c;
    if (fail_allocation)
    {
        return NULL;
    }
    allocation = malloc(n);
    return allocation;
}

static void release(void *c, void *p)
{
    (void)c;
    ++frees;
    free(p);
    allocation = NULL;
}

#define REQUIRE(x)                                        \
    do                                                    \
    {                                                     \
        if (!(x))                                         \
        {                                                 \
            fprintf(stderr, "MC_SHARED_ASSERT:%s\n", #x); \
            if (!launched)                                \
            {                                             \
                free(allocation);                         \
                allocation = NULL;                        \
            }                                             \
            exit(41);                                     \
        }                                                 \
    } while (0)

/* Worker arguments and results. gate/condition/ready/go order the "late"
 * case; barrier aligns the start of the "parallel" case. */
struct mc_context
{
    struct mc_shared *owner;
    pthread_barrier_t *barrier;
    unsigned out;
    int late, ready, go;
    pthread_mutex_t gate;
    pthread_cond_t condition;
};

static void *worker(void *arg)
{
    struct mc_context *c = arg;
    if (c->late)
    {
        if (pthread_mutex_lock(&c->gate))
        {
            return c;
        }
        c->ready = 1;
        if (pthread_cond_signal(&c->condition))
        {
            return c;
        }
        while (!c->go)
        {
            if (pthread_cond_wait(&c->condition, &c->gate))
            {
                return c;
            }
        }
        if (pthread_mutex_unlock(&c->gate))
        {
            return c;
        }
        if (mc_shared_add(c->owner, LATE_WORKER_AMOUNT) ||
            mc_shared_read(c->owner, &c->out))
        {
            return c;
        }
    }
    else
    {
        if (c->barrier)
        {
            int status = pthread_barrier_wait(c->barrier);
            if (status && status != PTHREAD_BARRIER_SERIAL_THREAD)
            {
                return c;
            }
        }
        for (unsigned i = 0; i < ADDS_PER_THREAD; ++i)
        {
            if (mc_shared_add(c->owner, 1))
            {
                return c;
            }
        }
    }
    if (mc_shared_drop(&c->owner))
    {
        return c;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int late = argc == 2 && !strcmp(argv[1], "late");
    int serial = argc == 2 && !strcmp(argv[1], "serial");
    struct mc_shared state = {0};
    struct mc_shared *main_owner = &state;
    struct mc_shared *extra = NULL;
    struct mc_shared *third = NULL;
    struct mc_shared *excess = NULL;
    struct mc_allocator a = {NULL, allocate, release};
    struct mc_context context = {0};
    pthread_t thread;
    pthread_barrier_t barrier;
    void *result;
    unsigned value = 99;
    fail_allocation = 1;
    REQUIRE(mc_shared_init(&state, a) == -1 && !state.initialized);
    fail_allocation = 0;
    REQUIRE(mc_shared_init(&state, a) == 0 && state.references == 1);
    REQUIRE(mc_shared_retain(&state, &extra) == 0 && state.references == 2);
    REQUIRE(mc_shared_retain(&state, &third) == 0 && state.references == 3);
    REQUIRE(mc_shared_retain(&state, &excess) == -1 && !excess);
    REQUIRE(mc_shared_drop(&third) == 0 && !third && state.references == 2);
    REQUIRE(mc_shared_finish(&state) == -1);
    context.owner = extra;
    extra = NULL;
    context.late = late;
    if (late)
    {
        REQUIRE(pthread_mutex_init(&context.gate, NULL) == 0 &&
                pthread_cond_init(&context.condition, NULL) == 0);
    }
    else if (!serial)
    {
        REQUIRE(pthread_barrier_init(&barrier, NULL, THREADS_AT_BARRIER) == 0);
        context.barrier = &barrier;
    }
    REQUIRE(pthread_create(&thread, NULL, worker, &context) == 0);
    launched = 1;
    if (late)
    {
        REQUIRE(pthread_mutex_lock(&context.gate) == 0);
        while (!context.ready)
        {
            REQUIRE(pthread_cond_wait(&context.condition, &context.gate) == 0);
        }
        REQUIRE(mc_shared_drop(&main_owner) == 0 && !main_owner && !frees);
        REQUIRE(mc_shared_finish(&state) == -1);
        context.go = 1;
        REQUIRE(pthread_cond_signal(&context.condition) == 0 &&
                pthread_mutex_unlock(&context.gate) == 0);
        REQUIRE(pthread_join(thread, &result) == 0 && !result &&
                !context.owner && context.out == LATE_WORKER_AMOUNT &&
                frees == 1);
        launched = 0;
        REQUIRE(pthread_cond_destroy(&context.condition) == 0 &&
                pthread_mutex_destroy(&context.gate) == 0);
    }
    else
    {
        if (serial)
        {
            REQUIRE(pthread_join(thread, &result) == 0 && !result);
            launched = 0;
        }
        else
        {
            int status = pthread_barrier_wait(&barrier);
            REQUIRE(status == 0 || status == PTHREAD_BARRIER_SERIAL_THREAD);
        }
        for (unsigned i = 0; i < ADDS_PER_THREAD; ++i)
        {
            REQUIRE(mc_shared_add(main_owner, 1) == 0);
        }
        if (!serial)
        {
            REQUIRE(pthread_join(thread, &result) == 0 && !result);
            launched = 0;
            REQUIRE(pthread_barrier_destroy(&barrier) == 0);
        }
        REQUIRE(mc_shared_read(main_owner, &value) == 0 &&
                value == 2 * ADDS_PER_THREAD && !context.owner && !frees);
        REQUIRE(mc_shared_drop(&main_owner) == 0 && !main_owner && frees == 1);
    }
    REQUIRE(mc_shared_finish(&state) == 0 && !state.initialized);
    puts(late     ? "MC_SHARED_LATE_PASS"
         : serial ? "MC_SHARED_SERIAL_PASS"
                  : "MC_SHARED_PARALLEL_PASS");
    return 0;
}
