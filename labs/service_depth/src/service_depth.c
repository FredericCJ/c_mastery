#include "service_depth.h"
int sd_load(SDMemory *m, const unsigned char *d, size_t n, int f, SDImage *o) {
  (void)m;
  (void)d;
  (void)n;
  (void)f;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_clone(SDMemory *m, SDImage *p, unsigned n, SDImage *o) {
  (void)m;
  (void)p;
  (void)n;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_write(SDMemory *m, SDImage *p, unsigned n, unsigned char v, int a) {
  (void)m;
  (void)p;
  (void)n;
  (void)v;
  (void)a;
  return SD_UNIMPLEMENTED;
}
int sd_release(SDMemory *m, SDImage *p) {
  (void)m;
  (void)p;
  return SD_UNIMPLEMENTED;
}
int sd_channel_init(SDChannel *q) {
  (void)q;
  return SD_UNIMPLEMENTED;
}
int sd_grant(const SDChannel *q, SDCap c, unsigned r, SDCap *o) {
  (void)q;
  (void)c;
  (void)r;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_send(SDChannel *q, SDCap c, const unsigned char *p, size_t n) {
  (void)q;
  (void)c;
  (void)p;
  (void)n;
  return SD_UNIMPLEMENTED;
}
int sd_receive(SDChannel *q, SDCap c, void *o, size_t n, size_t *r,
               SDValidate v, void *x) {
  (void)q;
  (void)c;
  (void)o;
  (void)n;
  (void)r;
  (void)v;
  (void)x;
  return SD_UNIMPLEMENTED;
}
int sd_channel_close(SDChannel *q, SDCap c) {
  (void)q;
  (void)c;
  return SD_UNIMPLEMENTED;
}
int sd_channel_reopen(SDChannel *q) {
  (void)q;
  return SD_UNIMPLEMENTED;
}
int sd_files_init(SDFiles *f, const SDEntry *e, unsigned n) {
  (void)f;
  (void)e;
  (void)n;
  return SD_UNIMPLEMENTED;
}
int sd_open(SDFiles *f, const char *n, SDHandle *o) {
  (void)f;
  (void)n;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_dup(SDFiles *f, SDHandle h, SDHandle *o) {
  (void)f;
  (void)h;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_seek(SDFiles *f, SDHandle h, unsigned w, int32_t n) {
  (void)f;
  (void)h;
  (void)w;
  (void)n;
  return SD_UNIMPLEMENTED;
}
int sd_read(SDFiles *f, SDHandle h, void *o, size_t c, size_t n, size_t *r,
            SDValidate v, void *x) {
  (void)f;
  (void)h;
  (void)o;
  (void)c;
  (void)n;
  (void)r;
  (void)v;
  (void)x;
  return SD_UNIMPLEMENTED;
}
int sd_close(SDFiles *f, SDHandle h) {
  (void)f;
  (void)h;
  return SD_UNIMPLEMENTED;
}
int sd_recover(const SDJournal *j, SDReceipt *o) {
  (void)j;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_apply(SDJournal *j, uint32_t n, int32_t d, unsigned f, int32_t *o) {
  (void)j;
  (void)n;
  (void)d;
  (void)f;
  (void)o;
  return SD_UNIMPLEMENTED;
}
