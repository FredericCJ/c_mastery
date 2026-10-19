#include "grace.h"
void ag_init(AGDomain *d, unsigned n) {
  (void)d;
  (void)n;
}
int ag_register(AGDomain *d, AGLease *l) {
  (void)d;
  (void)l;
  return AG_UNIMPLEMENTED;
}
int ag_enter(AGDomain *d, AGLease l) {
  (void)d;
  (void)l;
  return AG_UNIMPLEMENTED;
}
int ag_leave(AGDomain *d, AGLease l) {
  (void)d;
  (void)l;
  return AG_UNIMPLEMENTED;
}
int ag_depart(AGDomain *d, AGLease l) {
  (void)d;
  (void)l;
  return AG_UNIMPLEMENTED;
}
int ag_advance(AGDomain *d) {
  (void)d;
  return AG_UNIMPLEMENTED;
}
int ag_reclaimable(const AGDomain *d, unsigned e) {
  (void)d;
  (void)e;
  return 0;
}
void ag_store_init(AGStore *s, unsigned n, unsigned char v) {
  (void)s;
  (void)n;
  (void)v;
}
int ag_writer_acquire(AGStore *s, unsigned w) {
  (void)s;
  (void)w;
  return AG_UNIMPLEMENTED;
}
int ag_writer_release(AGStore *s, unsigned w) {
  (void)s;
  (void)w;
  return AG_UNIMPLEMENTED;
}
int ag_publish(AGStore *s, unsigned w, unsigned char v) {
  (void)s;
  (void)w;
  (void)v;
  return AG_UNIMPLEMENTED;
}
int ag_read(AGStore *s, AGLease l, AGBorrow *b) {
  (void)s;
  (void)l;
  (void)b;
  return AG_UNIMPLEMENTED;
}
int ag_copy(AGStore *s, AGBorrow b, unsigned char v[32]) {
  (void)s;
  (void)b;
  (void)v;
  return AG_UNIMPLEMENTED;
}
int ag_read_end(AGStore *s, AGBorrow b) {
  (void)s;
  (void)b;
  return AG_UNIMPLEMENTED;
}
unsigned ag_collect(AGStore *s) {
  (void)s;
  return 0;
}
ApiStatus ag_capture(AGStore *s, AGBorrow b, AGCallback c, void *ctx,
                     ApiHandle *h) {
  (void)s;
  (void)b;
  (void)c;
  (void)ctx;
  (void)h;
  return API_UNIMPLEMENTED;
}
ApiStatus ag_dispatch(AGStore *s) {
  (void)s;
  return API_UNIMPLEMENTED;
}
ApiStatus ag_remove(AGStore *s, ApiHandle h) {
  (void)s;
  (void)h;
  return API_UNIMPLEMENTED;
}
ApiStatus ag_destroy_callbacks(AGStore *s) {
  (void)s;
  return API_UNIMPLEMENTED;
}
