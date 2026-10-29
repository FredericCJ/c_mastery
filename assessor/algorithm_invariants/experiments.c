#include "invariants.h"
#include <stdlib.h>

struct ai_allocation
{
    size_t acquisitions, releases;
};

static int *take(void *p, size_t n)
{
    struct ai_allocation *a = p;
    int *v = malloc(n * sizeof *v);
    if (v)
    {
        ++a->acquisitions;
    }
    return v;
}

static void release(void *p, int *v)
{
    ++((struct ai_allocation *)p)->releases;
    free(v);
}

enum ai_status ai_growth(unsigned count, unsigned additive,
                         struct ai_growth *out)
{
    if (!out || count > 32 || additive > 1)
    {
        return AI_INVALID;
    }
    struct ai_allocation allocation = {0};
    struct cd_vector v;
    size_t worst = 0;
    if (cd_vector_init(&v, (struct cd_allocator){&allocation, take, release}) !=
        CD_OK)
    {
        return AI_INVALID;
    }
    for (unsigned i = 0; i < count; ++i)
    {
        size_t old = v.copies;
        if (additive && cd_vector_reserve(&v, v.length + 1) != CD_OK)
        {
            cd_vector_destroy(&v);
            return AI_INVALID;
        }
        if (cd_vector_insert(&v, v.length, (int)i) != CD_OK)
        {
            cd_vector_destroy(&v);
            return AI_INVALID;
        }
        size_t cost = v.copies - old + 1;
        if (cost > worst)
        {
            worst = cost;
        }
    }
    struct ai_growth result = {v.copies,   v.copies + count,        worst,
                               v.capacity, allocation.acquisitions, 0};
    cd_vector_destroy(&v);
    result.releases = allocation.releases;
#ifdef AI_MUTANT_GROWTH
    result.writes = count;
#endif
    *out = result;
    return AI_OK;
}

void ai_queues_init(struct ai_queues *q)
{
    cd_ring_init(&q->ring, q->backing, 3);
    cd_queue_init(&q->queue);
}

enum cd_status ai_queue_event(struct ai_queues *q, unsigned put, int value,
                              int *out)
{
    if (!q || !out || put > 1)
    {
        return CD_INVALID;
    }
    int a = 0, b = 0;
    enum cd_status ring, linked;
    size_t written = 0;
    if (put)
    {
        ring = cd_ring_push(&q->ring, &value, 1, 0, &written);
        linked = cd_queue_push(&q->queue, value, 0, 0);
    }
    else
    {
        ring = cd_ring_pop(&q->ring, &a, 1);
        linked = cd_queue_pop(&q->queue, &b);
    }
    if (ring != linked || (!put && ring == CD_OK && a != b))
    {
        return CD_FAILED;
    }
    if (!put && ring == CD_OK)
    {
        *out = a;
    }
    return ring;
}
