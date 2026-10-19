#include "os.h"
/* Learner-owned incomplete entry points. Preserve refusal without publication.
 */
OfStatus of_schedule(const os_job *a, size_t b, os_policy c, unsigned d,
                     unsigned e, OfSchedule *f) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  return OF_UNIMPLEMENTED;
}
OfStatus of_fair(unsigned a, unsigned b, const unsigned c[3], uint64_t d,
                 unsigned e, const OfEvent *f, size_t g, unsigned h,
                 OfFair *i) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  (void)g;
  (void)h;
  (void)i;
  return OF_UNIMPLEMENTED;
}
OfStatus of_pages(const OfReference *a, size_t b, unsigned c, OfPagePolicy d,
                  OfPages *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return OF_UNIMPLEMENTED;
}
OfStatus of_progress(OfTransfer *a, ptrdiff_t b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return OF_UNIMPLEMENTED;
}
