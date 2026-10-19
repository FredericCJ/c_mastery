#include "measure.h"
#include <stdio.h>
int cf_pool_measure(unsigned p, unsigned n, const char *csv) {
  (void)p;
  (void)n;
  (void)csv;
  fputs("CF_MEASURE_UNIMPLEMENTED:pool\n", stderr);
  return 41;
}
int cf_queue_measure(unsigned k, unsigned n, unsigned b, unsigned a, unsigned p,
                     unsigned t, const char *csv) {
  (void)k;
  (void)n;
  (void)b;
  (void)a;
  (void)p;
  (void)t;
  (void)csv;
  fputs("CF_MEASURE_UNIMPLEMENTED:queue\n", stderr);
  return 41;
}
int cf_rw_measure(unsigned k, unsigned p, unsigned w, const char *csv) {
  (void)k;
  (void)p;
  (void)w;
  (void)csv;
  fputs("CF_MEASURE_UNIMPLEMENTED:rw\n", stderr);
  return 41;
}
