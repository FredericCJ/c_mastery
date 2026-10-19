#include "rwlock.h"
/* Intentionally unsolved learner adapter. */
int cdr_init(CDRW *o,cmx_kernel *k,unsigned n){(void)o;(void)k;(void)n;return CMX_UNIMPLEMENTED;}
int cdr_register(CDRW *o,unsigned t,CDRHandle *h){(void)o;(void)t;(void)h;return CMX_UNIMPLEMENTED;}
int cdr_begin(CDRHandle h,unsigned m,cmx_deadline d,CDRTicket *t){(void)h;(void)m;(void)d;(void)t;return CMX_UNIMPLEMENTED;}
int cdr_result(CDRTicket t){(void)t;return CMX_UNIMPLEMENTED;}
int cdr_cancel(CDRTicket t){(void)t;return CMX_UNIMPLEMENTED;}
int cdr_unlock(CDRHandle h){(void)h;return CMX_UNIMPLEMENTED;}
int cdr_close(CDRW *o){(void)o;return CMX_UNIMPLEMENTED;}
int cdr_release(CDRHandle *h){(void)h;return CMX_UNIMPLEMENTED;}
int cdr_destroy(CDRW *o){(void)o;return CMX_UNIMPLEMENTED;}
