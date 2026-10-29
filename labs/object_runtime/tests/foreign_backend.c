#include "foreign_backend.h"

void *orx_backend_worker(void *context)
{
    struct orx_backend *b = context;
    size_t i;
    unsigned sum = 0;
    if (pthread_mutex_lock(&b->lock))
    {
        return b;
    }
    b->ready = 1;
    pthread_cond_broadcast(&b->changed);
    while (!b->go)
    {
        if (pthread_cond_wait(&b->changed, &b->lock))
        {
            pthread_mutex_unlock(&b->lock);
            return b;
        }
    }
    pthread_mutex_unlock(&b->lock);
    for (i = 0; i < b->length; i++)
    {
        sum += b->bytes[i];
    }
    b->sum = sum;
    return NULL;
}
