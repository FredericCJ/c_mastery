#ifndef CMASTERY_OBJECT_RUNTIME_EXT_H
#define CMASTERY_OBJECT_RUNTIME_EXT_H
#include "object_runtime.h"
#include "ownership.h"

/* Inert encodings: neither serialized handles nor binary32 payloads are
 * pointers. */
struct orx_handle {
  uint8_t slot, generation;
};
struct orx_value {
  unsigned tag;
  union {
    int32_t integer;
    uint32_t binary32;
    struct orx_handle handle;
  } as;
};
uint32_t orx_binary32(uint32_t bits);
enum or_status orx_value_decode(const unsigned char wire[6], struct orx_value *out);
enum or_status orx_value_encode(struct orx_value value, unsigned char wire[6]);
enum or_status orx_double_bits(double value, uint64_t *out);
enum or_status orx_bits_double(uint64_t bits, double *out);

struct orx_cell {
  unsigned char bytes[15];
  uint8_t length;
  struct orx_handle edge[2];
};
struct orx_entry {
  uint8_t live, generation, position, roots, pins;
};
struct orx_lease {
  struct ow_pin pin;
  struct orx_handle object;
  uint8_t active, generation;
};
struct orx_heap {
  struct orx_cell cell[8];
  struct orx_entry entry[8];
  struct orx_lease lease[4];
  uint8_t epoch;
};
struct orx_ticket {
  struct orx_heap *heap;
  uint8_t slot, generation;
};
struct orx_grant {
  struct orx_heap *heap;
  struct orx_handle object;
};
void orx_heap_init(struct orx_heap *heap);
enum or_status orx_new(struct orx_heap *heap, const unsigned char *bytes, size_t length,
                 struct orx_handle *out);
enum or_status orx_retain(struct orx_heap *heap, struct orx_handle object);
enum or_status orx_release(struct orx_heap *heap, struct orx_handle object);
enum or_status orx_read(const struct orx_heap *heap, struct orx_handle object, unsigned char out[15],
                  size_t *length);
enum or_status orx_edge(struct orx_heap *heap, struct orx_handle from, unsigned field, struct orx_handle to);
enum or_status orx_collect(struct orx_heap *heap, unsigned *reclaimed);
enum or_status orx_pack(struct orx_heap *heap, unsigned *moved);
enum or_status orx_import(struct orx_heap *heap, struct orx_value value, const struct orx_grant *grant,
                    struct orx_handle *out);
enum or_status orx_foreign_begin(struct orx_heap *heap, struct orx_handle object, uint64_t controller,
                           struct orx_ticket *ticket, const unsigned char **bytes,
                           size_t *length);
enum or_status orx_foreign_cancel(struct orx_ticket ticket, uint64_t controller);
/* Call only after the backend is joined/acknowledged and cannot access bytes
 * again. */
enum or_status orx_foreign_finish(struct orx_ticket ticket, uint64_t controller);

struct orx_intern {
  struct orx_heap *heap;
  struct v4_allocator allocator;
  int32_t *bucket;
  unsigned capacity, used;
  uint8_t observed_epoch;
};
enum or_status orx_intern_init(struct orx_intern *table, struct orx_heap *heap,
                         struct v4_allocator allocator);
enum or_status orx_intern_get(struct orx_intern *table, const unsigned char *bytes,
                        size_t length, struct orx_handle *out);
enum or_status orx_intern_reserve(struct orx_intern *table, unsigned capacity);
enum or_status orx_intern_sweep(struct orx_intern *table, unsigned *removed);
enum or_status orx_intern_close(struct orx_intern *table);
#endif
