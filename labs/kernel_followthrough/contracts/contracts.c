#include "contracts.h"
int kf_boot(const unsigned char *p, size_t n, KfBoot *o) {
  (void)p;
  (void)n;
  (void)o;
  return 0;
}
int kf_frames(cmk_frames *f, const cmk_boot *b, const KfSpan *r, size_t n) {
  (void)f;
  (void)b;
  (void)r;
  (void)n;
  return 0;
}
size_t kf_free(const KfPool *p) {
  (void)p;
  return 0;
}
int kf_allocate(KfPool *p, KfSpace *s, unsigned a, unsigned n, unsigned b) {
  (void)p;
  (void)s;
  (void)a;
  (void)n;
  (void)b;
  return 0;
}
int kf_reap_user(KfPool *p, KfSpace *s, unsigned i, const KfCpu *c) {
  (void)p;
  (void)s;
  (void)i;
  (void)c;
  return 0;
}
int kf_retire_slot(KfSpace *s, unsigned i, const KfCpu *c) {
  (void)s;
  (void)i;
  (void)c;
  return 0;
}
int kf_permission(const uint64_t e[4], unsigned w, unsigned x, unsigned u,
                  unsigned *f) {
  (void)e;
  (void)w;
  (void)x;
  (void)u;
  (void)f;
  return 0;
}
int kf_next(const unsigned *a, unsigned n, unsigned c) {
  (void)a;
  (void)n;
  (void)c;
  return -1;
}
int kf_weighted(const unsigned *a, const unsigned *w, unsigned n, int64_t *c) {
  (void)a;
  (void)w;
  (void)n;
  (void)c;
  return -1;
}
int kf_load_image(KfPool *p, KfSpace *s, const unsigned char *b, size_t n,
                  unsigned k) {
  (void)p;
  (void)s;
  (void)b;
  (void)n;
  (void)k;
  return 0;
}
int kf_archive_typed(const unsigned char *p, size_t n, const char *s,
                     unsigned t, const unsigned char **o, size_t *z) {
  (void)p;
  (void)n;
  (void)s;
  (void)t;
  (void)o;
  (void)z;
  return 0;
}
uint32_t kf_handle_open(KfHandles *h, unsigned s, unsigned r, unsigned a) {
  (void)h;
  (void)s;
  (void)r;
  (void)a;
  return 0;
}
int kf_handle_get(const KfHandles *h, uint64_t t, unsigned a, unsigned *r) {
  (void)h;
  (void)t;
  (void)a;
  (void)r;
  return 0;
}
int kf_handle_close(KfHandles *h, uint64_t t) {
  (void)h;
  (void)t;
  return 0;
}
uint32_t kf_delegate(const KfHandles *s, uint64_t t, KfHandles *d, unsigned i,
                     unsigned r) {
  (void)s;
  (void)t;
  (void)d;
  (void)i;
  (void)r;
  return 0;
}
int kf_line(KfLine *s, unsigned char c, char o[64], unsigned *n) {
  (void)s;
  (void)c;
  (void)o;
  (void)n;
  return 0;
}
