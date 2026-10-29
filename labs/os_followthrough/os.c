#include "os.h"
/* Learner-owned incomplete entry points. Preserve refusal without publication.
 */
enum of_status of_schedule(const os_job *a, size_t b, os_policy c, unsigned d,
                     unsigned e, struct of_schedule *f) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  return OF_UNIMPLEMENTED;
}
enum of_status of_fair(unsigned a, unsigned b, const unsigned c[3], uint64_t d,
                 unsigned e, const struct of_event *f, size_t g, unsigned h,
                 struct of_fair *i) {
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
enum of_status of_pages(const struct of_reference *a, size_t b, unsigned c, enum of_page_policy d,
                  struct of_pages *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return OF_UNIMPLEMENTED;
}
enum of_status of_progress(struct of_transfer *a, ptrdiff_t b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return OF_UNIMPLEMENTED;
}
