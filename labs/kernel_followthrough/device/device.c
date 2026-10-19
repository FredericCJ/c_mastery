#include "device.h"
/* Intentional learner implementation boundary. */
int kf_pci(const uint8_t c[256], const KfBar b[6], KfPci *o) {
  (void)c;
  (void)b;
  (void)o;
  return 0;
}
int kf_negotiate(uint64_t f, unsigned q, uint16_t n, const KfPci *p,
                 uint64_t *o) {
  (void)f;
  (void)q;
  (void)n;
  (void)p;
  (void)o;
  return 0;
}
void kf_queue_init(KfQueue *q, uint16_t i) {
  (void)q;
  (void)i;
}
int kf_submit(KfQueue *q, unsigned t, uint64_t s, const uint8_t *b,
              KfTicket *o) {
  (void)q;
  (void)t;
  (void)s;
  (void)b;
  (void)o;
  return 0;
}
int kf_complete(KfQueue *q, uint16_t i, uint32_t h, uint8_t s) {
  (void)q;
  (void)i;
  (void)h;
  (void)s;
  return 0;
}
int kf_timeout(KfQueue *q, KfTicket t) {
  (void)q;
  (void)t;
  return 0;
}
int kf_release(KfQueue *q, KfTicket t) {
  (void)q;
  (void)t;
  return 0;
}
int kf_reset(KfQueue *q, int a) {
  (void)q;
  (void)a;
  return 0;
}
int kf_page_sector(uint64_t r, unsigned b, uint32_t p, uint64_t *o) {
  (void)r;
  (void)b;
  (void)p;
  (void)o;
  return 0;
}
int kf_slice(uint64_t r, uint32_t b, uint32_t c, uint32_t l, uint32_t *o) {
  (void)r;
  (void)b;
  (void)c;
  (void)l;
  (void)o;
  return 0;
}
