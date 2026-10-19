#include "kernel.h"
enum ak_result ak_copy(AKPacket *p, size_t n, AKCopy f, void *c) {
  (void)p;
  (void)n;
  (void)f;
  (void)c;
  return AK_UNIMPLEMENTED;
}
enum ak_result ak_init(AKService *s, AKBackend b) {
  (void)s;
  (void)b;
  return AK_UNIMPLEMENTED;
}
enum ak_result ak_enqueue(AKService *s, ASValue v) {
  (void)s;
  (void)v;
  return AK_UNIMPLEMENTED;
}
enum ak_result ak_start(AKService *s) {
  (void)s;
  return AK_UNIMPLEMENTED;
}
enum ak_result ak_complete(AKService *s, ASValue *v) {
  (void)s;
  (void)v;
  return AK_UNIMPLEMENTED;
}
