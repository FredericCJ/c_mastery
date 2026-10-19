#include "barrier.h"
/* Intentionally incomplete learner scaffold. */
int cdb_init(CDBarrier *b,cmx_kernel *k,uint32_t m,unsigned n){(void)b;(void)k;(void)m;(void)n;return CMX_UNIMPLEMENTED;}
int cdb_register(CDBarrier *b,unsigned t,CDBMember *o){(void)b;(void)t;(void)o;return CMX_UNIMPLEMENTED;}
int cdb_arrive(CDBMember h,cmx_deadline d,CDBTicket *o){(void)h;(void)d;(void)o;return CMX_UNIMPLEMENTED;}
int cdb_result(CDBTicket t,unsigned *o){(void)t;(void)o;return CMX_UNIMPLEMENTED;}
int cdb_cancel(CDBTicket t){(void)t;return CMX_UNIMPLEMENTED;}
int cdb_stage(CDBarrier *b,uint32_t m){(void)b;(void)m;return CMX_UNIMPLEMENTED;}
int cdb_release(CDBMember *h){(void)h;return CMX_UNIMPLEMENTED;}
int cdb_apply(CDBarrier *b){(void)b;return CMX_UNIMPLEMENTED;}
int cdb_close(CDBarrier *b){(void)b;return CMX_UNIMPLEMENTED;}
int cdb_destroy(CDBarrier *b){(void)b;return CMX_UNIMPLEMENTED;}
