#include <pthread.h>
#include <stdio.h>
#ifndef RACE_REPAIRED
#define RACE_REPAIRED 0
#endif
static volatile unsigned count;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *increment(void *unused)
{
    (void)unused;
    for (unsigned i = 0; i < 10000; i++)
    {
        if (RACE_REPAIRED)
        {
            (void)pthread_mutex_lock(&lock);
        }
        count++;
        if (RACE_REPAIRED)
        {
            (void)pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    if (pthread_create(&a, NULL, increment, NULL) ||
        pthread_create(&b, NULL, increment, NULL))
    {
        return 2;
    }
    if (pthread_join(a, NULL) || pthread_join(b, NULL))
    {
        return 2;
    }
    printf("RACE_COMPLETED:%u\n", count);
    return RACE_REPAIRED && count != 20000 ? 1 : 0;
}
