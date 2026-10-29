#ifndef CMASTERY_V4_ALGO_H
#define CMASTERY_V4_ALGO_H
#include <stddef.h>
#include <stdint.h>

/* Allocation callbacks return genuine storage suitable for int32_t objects.
   reserve preserves all state on failure; successful growth invalidates
   borrows. */
struct v4_allocator
{
    void *context;
    int32_t *(*take)(void *, size_t);
    void (*release)(void *, int32_t *);
};

struct v4_array
{
    int32_t *data;
    size_t length, capacity;
    struct v4_allocator allocator;
};

int v4_reserve(struct v4_array *, size_t);

struct v4_list
{
    int32_t value[8];
    int next[8], head, tail;
    unsigned used;
};

int v4_list_push(struct v4_list *, int32_t);
int v4_list_pop(struct v4_list *, int32_t *);

/* Explicit byte-offset free list: no casts from character arrays to typed
 * objects. */
struct v4_free
{
    unsigned start[8], length[8], count;
};

int v4_free_take(struct v4_free *, unsigned, unsigned *);
int v4_free_return(struct v4_free *, unsigned, unsigned);
/* Input and scratch are disjoint typed arrays, each with the declared extent. */
int v4_radix(uint32_t *, size_t, uint32_t *, size_t);
/* 0/1 knapsack, at most 16 items and capacity 32; zero-weight items rejected.
 */
int v4_knapsack(const unsigned *, const unsigned *, size_t, unsigned,
                unsigned *);

struct v4_filter
{
    int32_t history[3];
    unsigned position;
    int32_t previous;
};

/* FIR [1,2,1]/4 with signed division toward zero, then IIR (old+fir)/2. */
int v4_filter(struct v4_filter *, int32_t, int32_t *, int32_t *);
#endif
