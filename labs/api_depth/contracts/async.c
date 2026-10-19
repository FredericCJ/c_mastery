/* Intentional learner scaffold. Complete according to ASYNC.md. */
#include "async.h"
void ac_operation_init(AcOperation *o) { (void)o; }
AcAsyncStatus ac_submit_v2(AcOperation *o,const unsigned char *p,size_t n,
 AcAllocate a,AcTerminal cb,void *ctx,unsigned in,uint32_t *t) {
 (void)o;(void)p;(void)n;(void)a;(void)cb;(void)ctx;(void)in;(void)t;
 return ACX_UNIMPLEMENTED;
}
AcAsyncStatus ac_cancel_v2(AcOperation *o,uint32_t t) {
 (void)o;(void)t;return ACX_UNIMPLEMENTED;
}
AcAsyncStatus ac_commit_v2(AcOperation *o,uint32_t t) {
 (void)o;(void)t;return ACX_UNIMPLEMENTED;
}
AcAsyncStatus ac_finish_v2(AcOperation *o,uint32_t t,unsigned a) {
 (void)o;(void)t;(void)a;return ACX_UNIMPLEMENTED;
}
const unsigned char *ac_box_data(const AcBox *b,size_t *n) {
 (void)b;(void)n;return 0;
}
void ac_box_drop(AcBox *b) { (void)b; }
