#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static uint32_t first = 7, second = ~7u;
static unsigned failed, observed;

static void *writer(void *arg)
{
    (void)arg;
    for (unsigned i = 0; i < 5000; ++i)
    {
        pthread_mutex_lock(&lock);
        first = i;
        second = ~i;
        pthread_mutex_unlock(&lock);
    }
    return 0;
}

static void *reader(void *arg)
{
    (void)arg;
    for (unsigned i = 0; i < 5000; ++i)
    {
        pthread_mutex_lock(&lock);
        failed |= second != ~first;
        ++observed;
        pthread_mutex_unlock(&lock);
    }
    return 0;
}

int main(void)
{
    pthread_t a, b;
    if (pthread_create(&a, 0, writer, 0) || pthread_create(&b, 0, reader, 0))
        return 2;
    if (pthread_join(a, 0) || pthread_join(b, 0) ||
        pthread_mutex_destroy(&lock))
        return 2;
    if (failed || observed != 5000)
        return 1;
    puts("XI_LOCK_PASS observed=5000 cooperating_host_threads_only");
    return 0;
}
