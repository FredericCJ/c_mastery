#include <pthread.h>
#include <stdio.h>
#include "atlas.h"
/* The indirect cross-thread access experiment uses the documented GCC extension. */
#ifdef OA_TLS_GLOBAL
static struct oa_record owned = {'t', 31, 1.0};
#else
static __thread struct oa_record owned = {'t', 31, 1.0};
#endif
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t changed = PTHREAD_COND_INITIALIZER;
static struct oa_record *published;
static int ready, finish, observed;

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
    if (owned.number != 31 || !published || published->number != 31 ||
        published == &owned)
    {
        result = 41;
    }
#ifndef OA_TLS_NOWRITE
    published->number = 73;
#endif
    published = NULL;
    finish = 1;
    pthread_cond_signal(&changed);
    pthread_mutex_unlock(&lock);
    if (pthread_join(thread, NULL))
    {
        return 3;
    }
    if (observed != 73 || owned.number != 31)
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
