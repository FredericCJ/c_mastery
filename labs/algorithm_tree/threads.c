#define _POSIX_C_SOURCE 200809L
#include "tree.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_barrier_t rendezvous;
static struct at_pool tree;
static unsigned broken;

struct at_operation
{
    int result;
};

static void must(int e)
{
    if (e)
    {
        fprintf(stderr, "AT_THREAD_ERROR:%d\n", e);
        exit(42);
    }
}

static void barrier(void)
{
    int e = pthread_barrier_wait(&rendezvous);
    if (e != 0 && e != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        must(e);
    }
}

static void *worker(void *arg)
{
    struct at_operation *op = arg;
    struct at_work w;
    if (broken)
    {
        must(pthread_mutex_lock(&lock));
        int absent = !at_contains(&tree, 9, 0, &w);
        must(pthread_mutex_unlock(&lock));
        barrier();
        must(pthread_mutex_lock(
            &lock)); /* Both accesses protected: logical atomicity failure, no deliberate C data race. */
        op->result = absent ? algo_avl_insert(&tree.tree, 9) : AT_DUPLICATE;
        must(pthread_mutex_unlock(&lock));
    }
    else
    {
        barrier();
        must(pthread_mutex_lock(&lock));
        op->result = at_insert(&tree, 9, 0, &w);
        must(pthread_mutex_unlock(&lock));
    }
    return NULL;
}

int main(void)
{
    for (unsigned mode = 0; mode < 2; ++mode)
    {
        broken = mode;
        at_init(&tree);
        struct at_operation a = {99}, b = {99};
        pthread_t first, second;
        must(pthread_barrier_init(&rendezvous, NULL, 2));
        must(pthread_create(&first, NULL, worker, &a));
        must(pthread_create(&second, NULL, worker, &b));
        must(pthread_join(first, NULL));
        must(pthread_join(second, NULL));
        must(pthread_barrier_destroy(&rendezvous));
        unsigned admitted = (a.result == ALGO_OK) + (b.result == ALGO_OK);
        struct at_certificate c;
        int height;
        if (at_certificate(&tree, &c) != ALGO_OK ||
            at_check(&c, 1, &height) != ALGO_OK || tree.tree.count != 1 ||
            admitted != (mode ? 2u : 1u))
        {
            fprintf(stderr,
                    "AT_THREAD_ASSERT:atomic reject-duplicate outcome\n");
            return 41;
        }
        printf("OBS threads mode=%s admitted=%u count=%u statuses=%d,%d\n",
               mode ? "split-check" : "serialized", admitted, tree.tree.count,
               a.result, b.result);
    }
    must(pthread_mutex_destroy(&lock));
    return 0;
}
