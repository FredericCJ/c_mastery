#ifndef CMASTERY_OBJECT_RUNTIME_EXT_H
#define CMASTERY_OBJECT_RUNTIME_EXT_H
#include "object_runtime.h"
#include "ownership.h"

/* Inert encodings: neither serialized handles nor binary32 payloads are
 * pointers. */
typedef struct {
  uint8_t slot, generation;
} ORXHandle;
typedef struct {
  unsigned tag;
  union {
    int32_t integer;
    uint32_t binary32;
    ORXHandle handle;
  } as;
} ORXValue;
uint32_t orx_binary32(uint32_t bits);
ORStatus orx_value_decode(const unsigned char wire[6], ORXValue *out);
ORStatus orx_value_encode(ORXValue value, unsigned char wire[6]);
ORStatus orx_double_bits(double value, uint64_t *out);
ORStatus orx_bits_double(uint64_t bits, double *out);

typedef struct {
  unsigned char bytes[15];
  uint8_t length;
  ORXHandle edge[2];
} ORXCell;
typedef struct {
  uint8_t live, generation, position, roots, pins;
} ORXEntry;
typedef struct {
  OwPin pin;
  ORXHandle object;
  uint8_t active, generation;
} ORXLease;
typedef struct {
  ORXCell cell[8];
  ORXEntry entry[8];
  ORXLease lease[4];
  uint8_t epoch;
} ORXHeap;
typedef struct {
  ORXHeap *heap;
  uint8_t slot, generation;
} ORXTicket;
typedef struct {
  ORXHeap *heap;
  ORXHandle object;
} ORXGrant;
void orx_heap_init(ORXHeap *heap);
ORStatus orx_new(ORXHeap *heap, const unsigned char *bytes, size_t length,
                 ORXHandle *out);
ORStatus orx_retain(ORXHeap *heap, ORXHandle object);
ORStatus orx_release(ORXHeap *heap, ORXHandle object);
ORStatus orx_read(const ORXHeap *heap, ORXHandle object, unsigned char out[15],
                  size_t *length);
ORStatus orx_edge(ORXHeap *heap, ORXHandle from, unsigned field, ORXHandle to);
ORStatus orx_collect(ORXHeap *heap, unsigned *reclaimed);
ORStatus orx_pack(ORXHeap *heap, unsigned *moved);
ORStatus orx_import(ORXHeap *heap, ORXValue value, const ORXGrant *grant,
                    ORXHandle *out);
ORStatus orx_foreign_begin(ORXHeap *heap, ORXHandle object, uint64_t controller,
                           ORXTicket *ticket, const unsigned char **bytes,
                           size_t *length);
ORStatus orx_foreign_cancel(ORXTicket ticket, uint64_t controller);
/* Call only after the backend is joined/acknowledged and cannot access bytes
 * again. */
ORStatus orx_foreign_finish(ORXTicket ticket, uint64_t controller);

typedef struct {
  ORXHeap *heap;
  V4Allocator allocator;
  int32_t *bucket;
  unsigned capacity, used;
  uint8_t observed_epoch;
} ORXIntern;
ORStatus orx_intern_init(ORXIntern *table, ORXHeap *heap,
                         V4Allocator allocator);
ORStatus orx_intern_get(ORXIntern *table, const unsigned char *bytes,
                        size_t length, ORXHandle *out);
ORStatus orx_intern_reserve(ORXIntern *table, unsigned capacity);
ORStatus orx_intern_sweep(ORXIntern *table, unsigned *removed);
ORStatus orx_intern_close(ORXIntern *table);
#endif
