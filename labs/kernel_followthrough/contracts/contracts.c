#include "contracts.h"
int kf_boot(const unsigned char *p, size_t n, struct kf_boot *o) {
  (void)p;
  (void)n;
  (void)o;
  return 0;
}
int kf_frames(cmk_frames *f, const cmk_boot *b, const struct kf_span *r, size_t n) {
  (void)f;
  (void)b;
  (void)r;
  (void)n;
  return 0;
}
size_t kf_free(const struct kf_pool *p) {
  (void)p;
  return 0;
}
int kf_allocate(struct kf_pool *p, struct kf_space *s, unsigned a, unsigned n, unsigned b) {
  (void)p;
  (void)s;
  (void)a;
  (void)n;
  (void)b;
  return 0;
}
int kf_reap_user(struct kf_pool *p, struct kf_space *s, unsigned i, const struct kf_cpu *c) {
  (void)p;
  (void)s;
  (void)i;
  (void)c;
  return 0;
}
int kf_retire_slot(struct kf_space *s, unsigned i, const struct kf_cpu *c) {
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
int kf_load_image(struct kf_pool *p, struct kf_space *s, const unsigned char *b, size_t n,
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
uint32_t kf_handle_open(struct kf_handles *h, unsigned s, unsigned r, unsigned a) {
  (void)h;
  (void)s;
  (void)r;
  (void)a;
  return 0;
}
int kf_handle_get(const struct kf_handles *h, uint64_t t, unsigned a, unsigned *r) {
  (void)h;
  (void)t;
  (void)a;
  (void)r;
  return 0;
}
int kf_handle_close(struct kf_handles *h, uint64_t t) {
  (void)h;
  (void)t;
  return 0;
}
uint32_t kf_delegate(const struct kf_handles *s, uint64_t t, struct kf_handles *d, unsigned i,
                     unsigned r) {
  (void)s;
  (void)t;
  (void)d;
  (void)i;
  (void)r;
  return 0;
}
int kf_line(struct kf_line *s, unsigned char c, char o[64], unsigned *n) {
  (void)s;
  (void)c;
  (void)o;
  (void)n;
  return 0;
}
