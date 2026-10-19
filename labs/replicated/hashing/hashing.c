#include "hashing.h"
int rph_init(RphSha *s) {
  (void)s;
  return RPH_UNIMPLEMENTED;
}
int rph_update(RphSha *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return RPH_UNIMPLEMENTED;
}
int rph_final(RphSha *s, unsigned char o[32]) {
  (void)s;
  (void)o;
  return RPH_UNIMPLEMENTED;
}
int rph_hash(const unsigned char *p, size_t n, unsigned char o[32]) {
  (void)p;
  (void)n;
  (void)o;
  return RPH_UNIMPLEMENTED;
}
int rph_hmac_init(RphHmac *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return RPH_UNIMPLEMENTED;
}
int rph_hmac_update(RphHmac *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return RPH_UNIMPLEMENTED;
}
int rph_hmac_final(RphHmac *s, unsigned char o[32]) {
  (void)s;
  (void)o;
  return RPH_UNIMPLEMENTED;
}
int rph_hmac(const unsigned char *k, size_t kn, const unsigned char *p,
             size_t n, unsigned char o[32]) {
  (void)k;
  (void)kn;
  (void)p;
  (void)n;
  (void)o;
  return RPH_UNIMPLEMENTED;
}
int rph_equal(const unsigned char a[32], const unsigned char b[32]) {
  (void)a;
  (void)b;
  return 0;
}
void rph_clear(void *p, size_t n) {
  (void)p;
  (void)n;
}
