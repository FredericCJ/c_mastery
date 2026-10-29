#include "command.h"
void sc_service_init(struct sc_service *s) { (void)s; }
enum sc_result sc_submit(struct sc_service *s, const struct sc_batch *b, struct sc_ticket *t) {
  (void)s;
  (void)b;
  (void)t;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_finish(struct sc_service *s, struct sc_ticket t, int c, const struct sc_authority *a,
                         uint32_t *p, uint32_t *q) {
  (void)s;
  (void)t;
  (void)c;
  (void)a;
  (void)p;
  (void)q;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_retry(struct sc_service *s, struct sc_ticket t) {
  (void)s;
  (void)t;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_control(struct sc_service *s) {
  (void)s;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_control_complete(struct sc_service *s) {
  (void)s;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_shutdown(struct sc_service *s) {
  (void)s;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_recover(struct sc_recovery *s, struct sc_resources r, int l) {
  (void)s;
  (void)r;
  (void)l;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_decode(const unsigned char *p, size_t n, const struct sc_authority *a,
                         int e, struct sc_batch *b, struct sc_meter *m) {
  (void)p;
  (void)n;
  (void)a;
  (void)e;
  (void)b;
  (void)m;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_revoke(struct sc_authority *a) {
  (void)a;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_apply(const struct sc_batch *b, const struct sc_authority *a, uint32_t *p,
                        uint32_t *q) {
  (void)b;
  (void)a;
  (void)p;
  (void)q;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_decimal(const unsigned char *p, size_t n, int z, unsigned *v,
                          char c[2]) {
  (void)p;
  (void)n;
  (void)z;
  (void)v;
  (void)c;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_extent(size_t o, size_t c, size_t w, size_t l, size_t *r) {
  (void)o;
  (void)c;
  (void)w;
  (void)l;
  (void)r;
  return SC_UNIMPLEMENTED;
}
