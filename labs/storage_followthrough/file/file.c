#include "file.h"
void sfi_init(struct sfi_op *o) { (void)o; }
int sfi_begin(struct sfi_op *o, struct sfi_io i, unsigned w, uint32_t p, const void *a, void *b,
              struct sfi_ticket *t) {
  (void)o;
  (void)i;
  (void)w;
  (void)p;
  (void)a;
  (void)b;
  (void)t;
  return SFI_UNIMPLEMENTED;
}
int sfi_step(struct sfi_ticket t, unsigned r, unsigned b) {
  (void)t;
  (void)r;
  (void)b;
  return SFI_UNIMPLEMENTED;
}
int sfi_cancel(struct sfi_ticket t) {
  (void)t;
  return SFI_UNIMPLEMENTED;
}
int sfi_release(struct sfi_ticket t) {
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
