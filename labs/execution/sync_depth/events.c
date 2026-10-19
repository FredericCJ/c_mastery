#include "events.h"
/* Deliberately incomplete learner scaffold. Implement EVENTS.md. */
int cde_init(CDEvents *e, cmx_kernel *k) { (void)e; (void)k; return CMX_INVALID; }
int cde_wait(CDEvents *e, unsigned t, uint32_t m, unsigned a, unsigned c,
             cmx_deadline d) {
  (void)e; (void)t; (void)m; (void)a; (void)c; (void)d; return CMX_INVALID;
}
int cde_result(CDEvents *e, unsigned t, uint32_t *o) {
  (void)e; (void)t; (void)o; return CMX_INVALID;
}
int cde_set(CDEvents *e, uint32_t b) { (void)e; (void)b; return CMX_INVALID; }
int cde_clear(CDEvents *e, uint32_t b) { (void)e; (void)b; return CMX_INVALID; }
int cde_close(CDEvents *e) { (void)e; return CMX_INVALID; }
int cde_destroy(CDEvents *e) { (void)e; return CMX_INVALID; }
void cde_sequence_init(CDESequence *s) { (void)s; }
int cde_sequence_publish(CDESequence *s) { (void)s; return CMX_INVALID; }
int cde_sequence_read(const CDESequence *s, CDECursor *c, uint64_t *d,
                       uint64_t *m) {
  (void)s; (void)c; (void)d; (void)m; return CMX_INVALID;
}
