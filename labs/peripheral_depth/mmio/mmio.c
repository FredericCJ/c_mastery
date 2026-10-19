#include "mmio.h"
int pm_access(PmModel *m, unsigned o, unsigned w, unsigned wr, uint32_t *v) {
  (void)m;
  (void)o;
  (void)w;
  (void)wr;
  (void)v;
  return -1;
}
int pm_gpio(FwIo io, unsigned s, unsigned c) {
  (void)io;
  (void)s;
  (void)c;
  return -1;
}
int pm_begin(PmRequest *r, unsigned char b, unsigned n) {
  (void)r;
  (void)b;
  (void)n;
  return -1;
}
int pm_step(PmRequest *r, PmModel *m) {
  (void)r;
  (void)m;
  return -1;
}
int pm_filter_init(PmFilter *d, unsigned n, uint32_t t, uint32_t g) {
  (void)d;
  (void)n;
  (void)t;
  (void)g;
  return -1;
}
int pm_sample(PmFilter *d, unsigned b, uint32_t t, unsigned *c) {
  (void)d;
  (void)b;
  (void)t;
  (void)c;
  return -1;
}
int pm_line(unsigned d, unsigned o, unsigned l, unsigned p) {
  (void)d;
  (void)o;
  (void)l;
  (void)p;
  return -1;
}

int pm_irq_event(PmInterrupt *q, PmModel *m, uint32_t now) {
  (void)q;
  (void)m;
  (void)now;
  return -1;
}
int pm_irq_begin(PmInterrupt *q, PmModel *m, unsigned char b, uint32_t now,
                 uint32_t ticks) {
  (void)q;
  (void)m;
  (void)b;
  (void)now;
  (void)ticks;
  return -1;
}
