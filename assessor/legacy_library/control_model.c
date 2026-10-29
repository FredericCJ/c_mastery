/* Finite RC11 counterexample uses atomic payload so weak ordering has a
   defined safety violation rather than relying on a non-atomic data race. */
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
static atomic_int ready, payload;

static void *producer(void *unused)
{
    (void)unused;
    atomic_store_explicit(&payload, 37, memory_order_relaxed);
#ifdef LG_WEAK_RELEASE
    atomic_store_explicit(&ready, 1, memory_order_relaxed);
#else
    atomic_store_explicit(&ready, 1, memory_order_release);
#endif
    return 0;
}

static void *consumer(void *unused)
{
    (void)unused;
#ifdef LG_WEAK_ACQUIRE
    int seen = atomic_load_explicit(&ready, memory_order_relaxed);
#else
    int seen = atomic_load_explicit(&ready, memory_order_acquire);
#endif
    if (seen)
        assert(atomic_load_explicit(&payload, memory_order_relaxed) == 37);
    return 0;
}

int main(void)
{
    pthread_t a, b;
    if (pthread_create(&a, 0, producer, 0) ||
        pthread_create(&b, 0, consumer, 0))
        return 1;
    pthread_join(a, 0);
    pthread_join(b, 0);
    return 0;
}
