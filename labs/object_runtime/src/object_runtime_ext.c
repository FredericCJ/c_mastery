#include "object_runtime_ext.h"
/* Learner implementations: preserve the published contracts and replace each
 * stub. */
uint32_t orx_binary32(uint32_t bits) {
  (void)bits;
  return 0;
}
ORStatus orx_value_decode(const unsigned char wire[6], ORXValue *out) {
  (void)wire;
  (void)out;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_value_encode(ORXValue value, unsigned char wire[6]) {
  (void)value;
  (void)wire;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_double_bits(double value, uint64_t *out) {
  (void)value;
  (void)out;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_bits_double(uint64_t bits, double *out) {
  (void)bits;
  (void)out;
  return OR_UNIMPLEMENTED;
}
void orx_heap_init(ORXHeap *heap) { (void)heap; }
ORStatus orx_new(ORXHeap *heap, const unsigned char *bytes, size_t length,
                 ORXHandle *out) {
  (void)heap;
  (void)bytes;
  (void)length;
  (void)out;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_retain(ORXHeap *heap, ORXHandle object) {
  (void)heap;
  (void)object;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_release(ORXHeap *heap, ORXHandle object) {
  (void)heap;
  (void)object;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_read(const ORXHeap *heap, ORXHandle object, unsigned char out[15],
                  size_t *length) {
  (void)heap;
  (void)object;
  (void)out;
  (void)length;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_edge(ORXHeap *heap, ORXHandle from, unsigned field, ORXHandle to) {
  (void)heap;
  (void)from;
  (void)field;
  (void)to;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_collect(ORXHeap *heap, unsigned *reclaimed) {
  (void)heap;
  (void)reclaimed;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_pack(ORXHeap *heap, unsigned *moved) {
  (void)heap;
  (void)moved;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_import(ORXHeap *heap, ORXValue value, const ORXGrant *grant,
                    ORXHandle *out) {
  (void)heap;
  (void)value;
  (void)grant;
  (void)out;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_foreign_begin(ORXHeap *heap, ORXHandle object, uint64_t controller,
                           ORXTicket *ticket, const unsigned char **bytes,
                           size_t *length) {
  (void)heap;
  (void)object;
  (void)controller;
  (void)ticket;
  (void)bytes;
  (void)length;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_foreign_cancel(ORXTicket ticket, uint64_t controller) {
  (void)ticket;
  (void)controller;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_foreign_finish(ORXTicket ticket, uint64_t controller) {
  (void)ticket;
  (void)controller;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_intern_init(ORXIntern *table, ORXHeap *heap,
                         V4Allocator allocator) {
  (void)table;
  (void)heap;
  (void)allocator;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_intern_get(ORXIntern *table, const unsigned char *bytes,
                        size_t length, ORXHandle *out) {
  (void)table;
  (void)bytes;
  (void)length;
  (void)out;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_intern_reserve(ORXIntern *table, unsigned capacity) {
  (void)table;
  (void)capacity;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_intern_sweep(ORXIntern *table, unsigned *removed) {
  (void)table;
  (void)removed;
  return OR_UNIMPLEMENTED;
}
ORStatus orx_intern_close(ORXIntern *table) {
  (void)table;
  return OR_UNIMPLEMENTED;
}
