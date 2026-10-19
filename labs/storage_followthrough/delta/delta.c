#include "delta.h"
int sd_format(store_io i) {
  (void)i;
  return SD_UNIMPLEMENTED;
}
int sd_open(SdStore *s, store_io i, unsigned f) {
  (void)s;
  (void)i;
  (void)f;
  return SD_UNIMPLEMENTED;
}
int sd_begin(SdStore *s) {
  (void)s;
  return SD_UNIMPLEMENTED;
}
int sd_write(SdStore *s, unsigned p, size_t o, const void *b, size_t n) {
  (void)s;
  (void)p;
  (void)o;
  (void)b;
  (void)n;
  return SD_UNIMPLEMENTED;
}
int sd_read(SdStore *s, unsigned p, void *b) {
  (void)s;
  (void)p;
  (void)b;
  return SD_UNIMPLEMENTED;
}
int sd_commit(SdStore *s) {
  (void)s;
  return SD_UNIMPLEMENTED;
}
int sd_checkpoint(SdStore *s) {
  (void)s;
  return SD_UNIMPLEMENTED;
}
void sd_abort(SdStore *s) { (void)s; }
