#include "common.h"

int hs_queue_init(struct hs_queue *q)
{
    if (!q)
        return -1;
    q->head = q->count = 0;
    q->closed = 0;
    if (pthread_mutex_init(&q->mutex, 0))
        return -1;
    if (pthread_cond_init(&q->readable, 0))
    {
        pthread_mutex_destroy(&q->mutex);
        return -1;
    }
    if (pthread_cond_init(&q->writable, 0))
    {
        pthread_cond_destroy(&q->readable);
        pthread_mutex_destroy(&q->mutex);
        return -1;
    }
    return 0;
}

int hs_queue_push(struct hs_queue *q, int value)
{
    if (!q)
        return -1;
    pthread_mutex_lock(&q->mutex);
    while (q->count == 4 && !q->closed)
        pthread_cond_wait(&q->writable, &q->mutex);
    if (q->closed)
    {
        pthread_mutex_unlock(&q->mutex);
        return -1;
    }
    q->data[(q->head + q->count) % 4] = value;
    q->count++;
    pthread_cond_signal(&q->readable);
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

int hs_queue_pop(struct hs_queue *q, int *value)
{
    if (!q || !value)
        return -1;
    pthread_mutex_lock(&q->mutex);
    while (!q->count && !q->closed)
        pthread_cond_wait(&q->readable, &q->mutex);
    if (!q->count)
    {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    *value = q->data[q->head];
    q->head = (q->head + 1) % 4;
    q->count--;
    pthread_cond_signal(&q->writable);
    pthread_mutex_unlock(&q->mutex);
    return 1;
}

int hs_queue_close(struct hs_queue *q)
{
    if (!q)
        return -1;
    pthread_mutex_lock(&q->mutex);
    q->closed = 1;
    pthread_cond_broadcast(&q->readable);
    pthread_cond_broadcast(&q->writable);
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

int hs_queue_destroy(struct hs_queue *q)
{
    if (!q)
        return -1;
    int a = pthread_cond_destroy(&q->readable),
        b = pthread_cond_destroy(&q->writable),
        c = pthread_mutex_destroy(&q->mutex);
    return a || b || c ? -1 : 0;
}
