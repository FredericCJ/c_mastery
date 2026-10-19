#include "file.h"
void sfi_init(SfiOp *o) { (void)o; }
int sfi_begin(SfiOp *o, SfiIo i, unsigned w, uint32_t p, const void *a, void *b,
              SfiTicket *t) {
  (void)o;
  (void)i;
  (void)w;
  (void)p;
  (void)a;
  (void)b;
  (void)t;
  return SFI_UNIMPLEMENTED;
}
int sfi_step(SfiTicket t, unsigned r, unsigned b) {
  (void)t;
  (void)r;
  (void)b;
  return SFI_UNIMPLEMENTED;
}
int sfi_cancel(SfiTicket t) {
  (void)t;
  return SFI_UNIMPLEMENTED;
}
int sfi_release(SfiTicket t) {
  (void)t;
  return SFI_UNIMPLEMENTED;
}
int sfi_create(int d, const char *n, uint32_t p, store_file *o) {
  (void)d;
  (void)n;
  (void)p;
  (void)o;
  return SFI_UNIMPLEMENTED;
}
int sfi_close(store_file *f) {
  (void)f;
  return SFI_UNIMPLEMENTED;
}
