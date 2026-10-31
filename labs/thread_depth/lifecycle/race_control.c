/* CM-SPECIMEN: defective by design; classify the defect before running
   (see ../atomics/CONTRACTS.md). Exists for detector sensitivity only: no
   output oracle. */
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdlib.h>
static pthread_barrier_t start;
static volatile unsigned shared;

static void *worker(void *arg)
{
    (void)arg;
    int e = pthread_barrier_wait(&start);
    if (e && e != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        abort();
    }
    for (unsigned i = 0; i < 1000; i++)
    {
        shared++;
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    if (pthread_barrier_init(&start, NULL, 2) ||
        pthread_create(&a, NULL, worker, NULL) ||
        pthread_create(&b, NULL, worker, NULL))
    {
        return 2;
    }
    if (pthread_join(a, NULL) || pthread_join(b, NULL) ||
        pthread_barrier_destroy(&start))
    {
        return 3;
    }
    return 0;
}
