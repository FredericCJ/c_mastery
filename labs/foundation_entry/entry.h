#ifndef FE_ENTRY_H
#define FE_ENTRY_H
#include "foundation.h"
#include <stddef.h>
#include <stdint.h>
typedef enum {
  FE_OK,
  FE_INVALID,
  FE_RANGE,
  FE_FULL,
  FE_ALLOC,
  FE_EMPTY,
  FE_NOT_IMPLEMENTED
} FeStatus;
typedef struct {
  unsigned entering;
  size_t index;
  int32_t value;
} FeEvent;
typedef struct {
  FeEvent event[34];
  size_t count, maximum_live;
} FeTrace;
typedef struct {
  void *context;
  int32_t *(*take)(void *, size_t);
  void (*release)(void *, int32_t *);
} FeAllocator;
/* All pointed-to control objects are live/disjoint. After scalar admission,
 * n must not exceed the actual accessible array extents. In particular the
 * staged byte-count rejection precedes access and allocation. Fixed and
 * staged prefixes allow arbitrary input/output overlap;
 * two-pass prefixes require disjoint arrays or exactly identical start
 * pointers. Range/allocation failures preserve output. Trace is an observation
 * output: admitted recursion/stack calls reset it and retain partial failure
 * history. */
FeStatus fe_fixed(const int32_t *, size_t, int32_t *, size_t);
FeStatus fe_recursive(const int32_t *, size_t, int32_t *, FeTrace *);
FeStatus fe_explicit_stack(const int32_t *, size_t, int32_t *, FeTrace *);
FeStatus fe_two_pass(const int32_t *, size_t, int32_t *, size_t);
FeStatus fe_staged(const int32_t *, size_t, int32_t *, size_t, FeAllocator);
FeStatus fe_maximum(const int32_t *, size_t, int32_t *);
FeStatus fe_encode24(uint32_t, unsigned char *, size_t);
int fe_any(uint8_t, uint8_t);
int fe_all(uint8_t, uint8_t);
int fe_forbidden_clear(uint8_t, uint8_t);
int32_t fe_saturating_add(int32_t, int32_t);
#endif
