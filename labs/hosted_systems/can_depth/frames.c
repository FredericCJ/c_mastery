/* Intentionally unfinished learner adapter. */
#include "frames.h"
int ca_decode(const void *p, size_t n, int flags, struct ca_frame *out) {
  (void)p;
  (void)n;
  (void)flags;
  (void)out;
  return -1;
}
int ca_encode(const struct ca_frame *f, struct can_frame *out) {
  (void)f;
  (void)out;
  return -1;
}
int ca_filter(uint32_t id, unsigned e, unsigned r, struct can_filter *out) {
  (void)id;
  (void)e;
  (void)r;
  (void)out;
  return -1;
}
int ca_accept(const struct ca_frame *f, const struct can_filter *filter,
              uint32_t errors) {
  (void)f;
  (void)filter;
  (void)errors;
  return -1;
}
int ca_loopback(unsigned s, unsigned r, unsigned enabled, unsigned own,
                const struct ca_frame *f, const struct can_filter *filter,
                uint32_t errors, struct ca_frame *out) {
  (void)s;
  (void)r;
  (void)enabled;
  (void)own;
  (void)f;
  (void)filter;
  (void)errors;
  (void)out;
  return -1;
}
