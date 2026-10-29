#define _POSIX_C_SOURCE 200809L
#include "checkpoint_memory.h"
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef P1_EFFECT
#define P1_EFFECT 0
#endif
struct p1_effects
{
    unsigned comparisons, logs, heap, waits, releases;
};

struct p1_hooks
{
    void *context;
    int (*compare)(void *, int, int);
    void (*log)(void *, int);
};

/* The signatures contain no effect information. These original teaching
   callbacks deliberately violate a claimed no-allocation/no-wait contract. */
static int compare(void *context, int a, int b)
{
    struct p1_effects *e = context;
    ++e->comparisons;
    if (P1_EFFECT == 1)
    {
        volatile unsigned char *p = malloc(1);
        if (!p)
            exit(2);
        ++e->heap;
        *p = 17;
        free((void *)p);
        ++e->releases;
    }
    return (a > b) - (a < b);
}

static void logger(void *context, int value)
{
    struct p1_effects *e = context;
    (void)value;
    ++e->logs;
    if (P1_EFFECT == 2)
    {
        ++e->waits;
        if (poll(NULL, 0, 1) != 0)
            exit(2);
    }
}

static unsigned consume(struct cpm_pool *p, struct cpm_ring *q,
                        struct p1_hooks hooks)
{
    unsigned matches = 0;
    struct cpm_handle h;
    const struct cpm_record *r;
    while (cpm_pop(q, &h) == CPM_OK)
    {
        if (cpm_lookup(p, h, &r) != CPM_OK)
            exit(2);
        int same = hooks.compare(hooks.context, r->value, 22) == 0;
        matches += (unsigned)same;
        hooks.log(hooks.context, same);
        if (cpm_release(p, h) != CPM_OK)
            exit(2);
    }
    return matches;
}

int main(void)
{
    struct cpm_pool p;
    struct cpm_ring q;
    cpm_pool_init(&p);
    cpm_ring_init(&q);
    int values[] = {11, 22, 33, 44};
    size_t accepted = 0;
    struct p1_effects effects = {0};
    if (cpm_prefix(&p, &q, values, 4, &accepted) != CPM_OK || accepted != 4)
        return 2;
    unsigned matches =
        consume(&p, &q, (struct p1_hooks){&effects, compare, logger});
    if (matches != 1 || effects.comparisons != 4 || effects.logs != 4 ||
        q.count)
        return 2;
    for (unsigned i = 0; i < 4; i++)
        if (p.state[i] != 0)
            return 2;
    if (effects.heap || effects.waits)
    {
        printf(
            "P1_EFFECT_REJECT heap=%u waits=%u releases=%u comparisons=4 logs=4\n",
            effects.heap, effects.waits, effects.releases);
        return 1;
    }
    puts("P1_EFFECT_PASS heap=0 waits=0 comparisons=4 logs=4");
    return 0;
}
