#ifndef CMASTERY_V4_ALGO_H
#define CMASTERY_V4_ALGO_H
#include <stddef.h>
#include <stdint.h>
/* Allocation callbacks return genuine storage suitable for int32_t objects.
   reserve preserves all state on failure; successful growth invalidates
   borrows. */
typedef struct {
  void *context;
  int32_t *(*take)(void *, size_t);
  void (*release)(void *, int32_t *);
} V4Allocator;
typedef struct {
  int32_t *data;
  size_t length, capacity;
  V4Allocator allocator;
} V4Array;
int v4_reserve(V4Array *, size_t);
typedef struct {
  int32_t value[8];
  int next[8], head, tail;
  unsigned used;
} V4List;
int v4_list_push(V4List *, int32_t);
int v4_list_pop(V4List *, int32_t *);
/* Explicit byte-offset free list: no casts from character arrays to typed
 * objects. */
typedef struct {
  unsigned start[8], length[8], count;
} V4Free;
int v4_free_take(V4Free *, unsigned, unsigned *);
int v4_free_return(V4Free *, unsigned, unsigned);
/* Input and scratch are disjoint typed arrays, each with the declared extent. */
int v4_radix(uint32_t *, size_t, uint32_t *, size_t);
/* 0/1 knapsack, at most 16 items and capacity 32; zero-weight items rejected.
 */
int v4_knapsack(const unsigned *, const unsigned *, size_t, unsigned,
                unsigned *);
typedef struct {
  int32_t history[3];
  unsigned position;
  int32_t previous;
} V4Filter;
/* FIR [1,2,1]/4 with signed division toward zero, then IIR (old+fir)/2. */
int v4_filter(V4Filter *, int32_t, int32_t *, int32_t *);
#endif
