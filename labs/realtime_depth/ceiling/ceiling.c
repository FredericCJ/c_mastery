#include "ceiling.h"
/* Unsolved learner implementation; the actual completed CMX is imported
 * explicitly by the laboratory runner, never copied from assessor sources. */
enum cmx_result sc_init(SCWorld *w, const SCConfig *c, enum sc_mode m) {
  (void)w;
  (void)c;
  (void)m;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result sc_lock(SCWorld *w, unsigned t, unsigned r, cmx_deadline d,
                        enum sc_context c) {
  (void)w;
  (void)t;
  (void)r;
  (void)d;
  (void)c;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result sc_unlock(SCWorld *w, unsigned t, unsigned r,
                          enum sc_context c) {
  (void)w;
  (void)t;
  (void)r;
  (void)c;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result sc_poll(SCWorld *w, unsigned t) {
  (void)w;
  (void)t;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result sc_cancel(SCWorld *w, unsigned t) {
  (void)w;
  (void)t;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result sc_tick(SCWorld *w, uint32_t n) {
  (void)w;
  (void)n;
  return CMX_UNIMPLEMENTED;
}
enum cmx_result sc_set_base(SCWorld *w, unsigned t, unsigned p) {
  (void)w;
  (void)t;
  (void)p;
  return CMX_UNIMPLEMENTED;
}
int sc_invariant(const SCWorld *w) {
  (void)w;
  return 0;
}
int sc_choose(const SCWorld *w, unsigned m, int c) {
  (void)w;
  (void)m;
  (void)c;
  return CMX_NONE;
}
