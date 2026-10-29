#include "rational_units.h"
enum adq_status adq_scale(int64_t v, uint32_t n, uint32_t d, struct adq_result *o) {
  (void)v;
  (void)n;
  (void)d;
  (void)o;
  return ADQ_UNIMPLEMENTED;
}
enum adq_status adq_to_ticks(struct adq_microseconds v, struct adq_period p, struct adq_result *o) {
  (void)v;
  (void)p;
  (void)o;
  return ADQ_UNIMPLEMENTED;
}
enum adq_status adq_to_microseconds(struct adq_ticks v, struct adq_period p, struct adq_result *o) {
  (void)v;
  (void)p;
  (void)o;
  return ADQ_UNIMPLEMENTED;
}
