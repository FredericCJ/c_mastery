#include "notifications.h"
/* Unsolved learner scaffold: preserve the actual CMX producer. */
void cdn_init(CDNotify *n, cmx_kernel *k) { (void)n; (void)k; }
int cdn_attach(CDNotify *n,unsigned t,unsigned m,CDNHandle *h) { (void)n;(void)t;(void)m;(void)h;return CMX_INVALID; }
int cdn_post(CDNHandle h,uint32_t n,CDNPost *p) { (void)h;(void)n;(void)p;return CMX_INVALID; }
int cdn_wait(CDNHandle h,uint32_t n,cmx_deadline d) { (void)h;(void)n;(void)d;return CMX_INVALID; }
int cdn_result(CDNHandle h,uint32_t *n) { (void)h;(void)n;return CMX_INVALID; }
int cdn_cancel(CDNHandle h) { (void)h;return CMX_INVALID; }
int cdn_close(CDNHandle h) { (void)h;return CMX_INVALID; }
int cdn_detach(CDNHandle h) { (void)h;return CMX_INVALID; }
int cdn_fault(CDNHandle h,CDNFault *f) { (void)h;(void)f;return CMX_INVALID; }
int cdn_ack_fault(CDNHandle h,uint64_t s) { (void)h;(void)s;return CMX_INVALID; }
int cdn_invariant(const CDNotify *n) { (void)n;return 0; }
