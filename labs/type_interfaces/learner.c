#include "types.h"
int (*td_row_factory(void))[4] { return NULL; }
TdUnaryPtr td_factory(void) { return NULL; }
TdCallable td_wrapped_factory(void) { return (TdCallable){NULL}; }
struct TdOpaque *td_owner(unsigned i) {
  (void)i;
  return NULL;
}
TdStatus td_get(const struct TdOpaque *p, int *o) {
  (void)p;
  (void)o;
  return TD_UNFINISHED;
}
TdStatus td_add(struct TdOpaque *p, int v) {
  (void)p;
  (void)v;
  return TD_UNFINISHED;
}
TdStatus td_speed(TdMeters d, TdSeconds t, TdSpeed *o) {
  (void)d;
  (void)t;
  (void)o;
  return TD_UNFINISHED;
}
TdStatus td_distance(TdSpeed v, TdSeconds t, TdMeters *o) {
  (void)v;
  (void)t;
  (void)o;
  return TD_UNFINISHED;
}
int td_step(int v, void *c) {
  (void)v;
  (void)c;
  return 0;
}
int td_adapter(void *c, int v) {
  (void)v;
  (void)c;
  return 0;
}
TdStatus td_rows(const int (*a)[4], size_t n, TdDispatch f, void *c, int *o) {
  (void)a;
  (void)n;
  (void)f;
  (void)c;
  (void)o;
  return TD_UNFINISHED;
}
TdStatus td_submit(TdJob *j, TdDispatch f, void *c, int v) {
  (void)j;
  (void)f;
  (void)c;
  (void)v;
  return TD_UNFINISHED;
}
TdStatus td_run(TdJob *j, int *o) {
  (void)j;
  (void)o;
  return TD_UNFINISHED;
}
TdStatus td_cancel(TdJob *j) {
  (void)j;
  return TD_UNFINISHED;
}
TdStatus td_reset(TdJob *j) {
  (void)j;
  return TD_UNFINISHED;
}
