/*
 * Thread-local storage experiment (GCC __thread extension, GNU17 + POSIX).
 *
 * The worker publishes the address of its own `owned` record to the main
 * thread, then waits until the main thread sets `finish`. `lock` protects
 * published, ready, finish and observed; all waiting is on the condition
 * variable `changed`.
 *
 * Exit status: 0 success (marker OA_TLS_PASS:); 41 an identity or value
 * check failed (marker OA_TLS_ASSERT:); 2 thread creation failed; 3 join
 * failed.
 */
#include <pthread.h>
#include <stdio.h>
#include "atlas.h"

enum
{
    INITIAL_NUMBER = 31,
    WRITTEN_NUMBER = 73
};

#ifdef OA_TLS_GLOBAL
static struct oa_record owned = {'t', INITIAL_NUMBER, 1.0};
#else
static __thread struct oa_record owned = {'t', INITIAL_NUMBER, 1.0};
#endif
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t changed = PTHREAD_COND_INITIALIZER;
static struct oa_record *published; /* the worker's `owned`, while alive */
static int ready; /* set by the worker once published */
static int finish; /* set by main when the worker may exit */
static int observed; /* worker's own view of owned.number */

static void *worker(void *unused)
{
    (void)unused;
    pthread_mutex_lock(&lock);
    published = &owned;
    ready = 1;
    pthread_cond_signal(&changed);
    while (!finish)
    {
        pthread_cond_wait(&changed, &lock);
    }
    observed = owned.number;
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(void)
{
    pthread_t thread;
    int result = 0;
    if (pthread_create(&thread, NULL, worker, NULL))
    {
        return 2;
    }
    pthread_mutex_lock(&lock);
    while (!ready)
    {
        pthread_cond_wait(&changed, &lock);
    }
    if (owned.number != INITIAL_NUMBER || !published ||
        published->number != INITIAL_NUMBER || published == &owned)
    {
        result = 41;
    }
#ifndef OA_TLS_NOWRITE
    published->number = WRITTEN_NUMBER;
#endif
    /* Withdraw the pointer before the worker is allowed to finish. */
    published = NULL;
    finish = 1;
    pthread_cond_signal(&changed);
    pthread_mutex_unlock(&lock);
    if (pthread_join(thread, NULL))
    {
        return 3;
    }
    if (observed != WRITTEN_NUMBER || owned.number != INITIAL_NUMBER)
    {
        result = 41;
    }
    pthread_cond_destroy(&changed);
    pthread_mutex_destroy(&lock);
    if (!result)
    {
        puts(
            "OA_TLS_PASS:published only while creator alive; pointer cleared before exit");
    }
    else
    {
        puts("OA_TLS_ASSERT:identity or synchronized transfer");
    }
    return result;
}
