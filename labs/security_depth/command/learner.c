#include "command.h"
void sc_service_init(SCService *s) { (void)s; }
enum sc_result sc_submit(SCService *s, const SCBatch *b, SCTicket *t) {
  (void)s;
  (void)b;
  (void)t;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_finish(SCService *s, SCTicket t, int c, const SCAuthority *a,
                         uint32_t *p, uint32_t *q) {
  (void)s;
  (void)t;
  (void)c;
  (void)a;
  (void)p;
  (void)q;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_retry(SCService *s, SCTicket t) {
  (void)s;
  (void)t;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_control(SCService *s) {
  (void)s;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_control_complete(SCService *s) {
  (void)s;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_shutdown(SCService *s) {
  (void)s;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_recover(SCRecovery *s, SCResources r, int l) {
  (void)s;
  (void)r;
  (void)l;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_decode(const unsigned char *p, size_t n, const SCAuthority *a,
                         int e, SCBatch *b, SCMeter *m) {
  (void)p;
  (void)n;
  (void)a;
  (void)e;
  (void)b;
  (void)m;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_revoke(SCAuthority *a) {
  (void)a;
  return SC_UNIMPLEMENTED;
}
enum sc_result sc_apply(const SCBatch *b, const SCAuthority *a, uint32_t *p,
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
