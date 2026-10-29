/* Intentionally racy original diagnostic control. No portable C result is
 * asserted. Only an instrumented race diagnostic counts under this harness. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
static volatile unsigned value;
static pthread_barrier_t gate;

static void *writer(void *arg)
{
    (void)arg;
    pthread_barrier_wait(&gate);
    for (unsigned i = 0; i < 1000; ++i)
    {
        ++value;
    }
    return 0;
}

int main(void)
{
    pthread_t a, b;
    if (pthread_barrier_init(&gate, 0, 3) || pthread_create(&a, 0, writer, 0) ||
        pthread_create(&b, 0, writer, 0))
    {
        return 2;
    }
    pthread_barrier_wait(&gate);
    if (pthread_join(a, 0) || pthread_join(b, 0) ||
        pthread_barrier_destroy(&gate))
    {
        return 2;
    }
    return 0;
}
