#ifndef FE_ENTRY_H
#define FE_ENTRY_H
#include "foundation.h"
#include <stddef.h>
#include <stdint.h>

enum fe_status
{
    FE_OK,
    FE_INVALID,
    FE_RANGE,
    FE_FULL,
    FE_ALLOC,
    FE_EMPTY,
    FE_NOT_IMPLEMENTED
};

struct fe_event
{
    unsigned entering;
    size_t index;
    int32_t value;
};

struct fe_trace
{
    struct fe_event event[34];
    size_t count, maximum_live;
};

struct fe_allocator
{
    void *context;
    int32_t *(*take)(void *, size_t);
    void (*release)(void *, int32_t *);
};
/* All pointed-to control objects are live/disjoint. After scalar admission,
 * n must not exceed the actual accessible array extents. In particular the
 * staged byte-count rejection precedes access and allocation. Fixed and
 * staged prefixes allow arbitrary input/output overlap;
 * two-pass prefixes require disjoint arrays or exactly identical start
 * pointers. Range/allocation failures preserve output. Trace is an observation
 * output: admitted recursion/stack calls reset it and retain partial failure
 * history. */
enum fe_status fe_fixed(const int32_t *, size_t, int32_t *, size_t);
enum fe_status fe_recursive(const int32_t *, size_t, int32_t *,
                            struct fe_trace *);
enum fe_status fe_explicit_stack(const int32_t *, size_t, int32_t *,
                                 struct fe_trace *);
enum fe_status fe_two_pass(const int32_t *, size_t, int32_t *, size_t);
enum fe_status fe_staged(const int32_t *, size_t, int32_t *, size_t,
                         struct fe_allocator);
enum fe_status fe_maximum(const int32_t *, size_t, int32_t *);
enum fe_status fe_encode24(uint32_t, unsigned char *, size_t);
int fe_any(uint8_t, uint8_t);
int fe_all(uint8_t, uint8_t);
int fe_forbidden_clear(uint8_t, uint8_t);
int32_t fe_saturating_add(int32_t, int32_t);
#endif
