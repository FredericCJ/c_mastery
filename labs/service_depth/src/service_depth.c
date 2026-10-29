#include "service_depth.h"
int sd_load(struct sd_memory *m, const unsigned char *d, size_t n, int f, struct sd_image *o) {
  (void)m;
  (void)d;
  (void)n;
  (void)f;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_clone(struct sd_memory *m, struct sd_image *p, unsigned n, struct sd_image *o) {
  (void)m;
  (void)p;
  (void)n;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_write(struct sd_memory *m, struct sd_image *p, unsigned n, unsigned char v, int a) {
  (void)m;
  (void)p;
  (void)n;
  (void)v;
  (void)a;
  return SD_UNIMPLEMENTED;
}
int sd_release(struct sd_memory *m, struct sd_image *p) {
  (void)m;
  (void)p;
  return SD_UNIMPLEMENTED;
}
int sd_channel_init(struct sd_channel *q) {
  (void)q;
  return SD_UNIMPLEMENTED;
}
int sd_grant(const struct sd_channel *q, struct sd_cap c, unsigned r, struct sd_cap *o) {
  (void)q;
  (void)c;
  (void)r;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_send(struct sd_channel *q, struct sd_cap c, const unsigned char *p, size_t n) {
  (void)q;
  (void)c;
  (void)p;
  (void)n;
  return SD_UNIMPLEMENTED;
}
int sd_receive(struct sd_channel *q, struct sd_cap c, void *o, size_t n, size_t *r,
               sd_validate_fn v, void *x) {
  (void)q;
  (void)c;
  (void)o;
  (void)n;
  (void)r;
  (void)v;
  (void)x;
  return SD_UNIMPLEMENTED;
}
int sd_channel_close(struct sd_channel *q, struct sd_cap c) {
  (void)q;
  (void)c;
  return SD_UNIMPLEMENTED;
}
int sd_channel_reopen(struct sd_channel *q) {
  (void)q;
  return SD_UNIMPLEMENTED;
}
int sd_files_init(struct sd_files *f, const struct sd_entry *e, unsigned n) {
  (void)f;
  (void)e;
  (void)n;
  return SD_UNIMPLEMENTED;
}
int sd_open(struct sd_files *f, const char *n, struct sd_handle *o) {
  (void)f;
  (void)n;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_dup(struct sd_files *f, struct sd_handle h, struct sd_handle *o) {
  (void)f;
  (void)h;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_seek(struct sd_files *f, struct sd_handle h, unsigned w, int32_t n) {
  (void)f;
  (void)h;
  (void)w;
  (void)n;
  return SD_UNIMPLEMENTED;
}
int sd_read(struct sd_files *f, struct sd_handle h, void *o, size_t c, size_t n, size_t *r,
            sd_validate_fn v, void *x) {
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
int sd_close(struct sd_files *f, struct sd_handle h) {
  (void)f;
  (void)h;
  return SD_UNIMPLEMENTED;
}
int sd_recover(const struct sd_journal *j, struct sd_receipt *o) {
  (void)j;
  (void)o;
  return SD_UNIMPLEMENTED;
}
int sd_apply(struct sd_journal *j, uint32_t n, int32_t d, unsigned f, int32_t *o) {
  (void)j;
  (void)n;
  (void)d;
  (void)f;
  (void)o;
  return SD_UNIMPLEMENTED;
}
