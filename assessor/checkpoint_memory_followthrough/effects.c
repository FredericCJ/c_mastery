/* CM-SPECIMEN: hidden-effect callbacks; with P1_EFFECT=1 or 2 the comparison
   or logging callback allocates or waits despite a claimed contract that
   forbids both, and nothing in its signature shows it. */
#define _POSIX_C_SOURCE 200809L
#include "checkpoint_memory.h"
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * P1_EFFECT selects the callback behaviour: 0 honest, 1 compare() allocates
 * from the heap, 2 logger() waits in poll(). Exit status: 0 contract held,
 * 1 contract violated (reported on stdout), 2 harness error.
 */
#ifndef P1_EFFECT
#define P1_EFFECT 0
#endif
enum
{
    EFFECT_HONEST = 0,
    EFFECT_HEAP = 1,
    EFFECT_WAIT = 2,
    EXIT_VIOLATION = 1,
    EXIT_HARNESS_ERROR = 2,
    OFFERED_VALUES = 4,
    MATCH_VALUE = 22,
    WAIT_MS = 1
};

struct p1_effects
{
    unsigned comparisons;
    unsigned logs;
    unsigned heap;
    unsigned waits;
    unsigned releases;
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
    if (P1_EFFECT == EFFECT_HEAP)
    {
        volatile unsigned char *p = malloc(1);
        if (!p)
        {
            exit(EXIT_HARNESS_ERROR);
        }
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
    if (P1_EFFECT == EFFECT_WAIT)
    {
        ++e->waits;
        if (poll(NULL, 0, WAIT_MS) != 0)
        {
            exit(EXIT_HARNESS_ERROR);
        }
    }
}

/* Drains the ring, comparing each value with MATCH_VALUE; returns matches. */
static unsigned consume(struct cpm_pool *p, struct cpm_ring *q,
                        struct p1_hooks hooks)
{
    unsigned matches = 0;
    struct cpm_handle h;
    const struct cpm_record *r;
    while (cpm_pop(q, &h) == CPM_OK)
    {
        if (cpm_lookup(p, h, &r) != CPM_OK)
        {
            exit(EXIT_HARNESS_ERROR);
        }
        int same = hooks.compare(hooks.context, r->value, MATCH_VALUE) == 0;
        matches += (unsigned)same;
        hooks.log(hooks.context, same);
        if (cpm_release(p, h) != CPM_OK)
        {
            exit(EXIT_HARNESS_ERROR);
        }
    }
    return matches;
}

int main(void)
{
    struct cpm_pool p;
    struct cpm_ring q;
    cpm_pool_init(&p);
    cpm_ring_init(&q);
    int values[OFFERED_VALUES] = {11, 22, 33, 44};
    size_t accepted = 0;
    struct p1_effects effects = {0};
    if (cpm_prefix(&p, &q, values, OFFERED_VALUES, &accepted) != CPM_OK ||
        accepted != OFFERED_VALUES)
    {
        return EXIT_HARNESS_ERROR;
    }
    unsigned matches =
        consume(&p, &q, (struct p1_hooks){&effects, compare, logger});
    if (matches != 1 || effects.comparisons != OFFERED_VALUES ||
        effects.logs != OFFERED_VALUES || q.count)
    {
        return EXIT_HARNESS_ERROR;
    }
    for (unsigned i = 0; i < OFFERED_VALUES; i++)
    {
        if (p.state[i] != 0)
        {
            return EXIT_HARNESS_ERROR;
        }
    }
    if (effects.heap || effects.waits)
    {
        printf(
            "P1_EFFECT_REJECT heap=%u waits=%u releases=%u comparisons=4 logs=4\n",
            effects.heap, effects.waits, effects.releases);
        return EXIT_VIOLATION;
    }
    puts("P1_EFFECT_PASS heap=0 waits=0 comparisons=4 logs=4");
    return 0;
}
