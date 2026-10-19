#include "once.h"
/* Intentionally unsolved learner adapter. */
int cdo_init(CDOnce *o,cmx_kernel *k,CDOAllocate a,CDODrop d,void *c){(void)o;(void)k;(void)a;(void)d;(void)c;return CMX_UNIMPLEMENTED;}
int cdo_register(CDOnce *o,unsigned t,CDOHandle *h){(void)o;(void)t;(void)h;return CMX_UNIMPLEMENTED;}
int cdo_enter(CDOHandle h,cmx_deadline d,CDOTicket *t){(void)h;(void)d;(void)t;return CMX_UNIMPLEMENTED;}
int cdo_allocate(CDOHandle h){(void)h;return CMX_UNIMPLEMENTED;}
int cdo_write(CDOHandle h,unsigned i,uint32_t v){(void)h;(void)i;(void)v;return CMX_UNIMPLEMENTED;}
int cdo_seal(CDOHandle h){(void)h;return CMX_UNIMPLEMENTED;}
int cdo_finish(CDOHandle h,unsigned s){(void)h;(void)s;return CMX_UNIMPLEMENTED;}
int cdo_result(CDOTicket t,CDOTable *o){(void)t;(void)o;return CMX_UNIMPLEMENTED;}
int cdo_read(CDOHandle h,CDOTable *o){(void)h;(void)o;return CMX_UNIMPLEMENTED;}
int cdo_cancel(CDOTicket t){(void)t;return CMX_UNIMPLEMENTED;}
int cdo_prepare_reset(CDOnce *o){(void)o;return CMX_UNIMPLEMENTED;}
int cdo_release(CDOHandle *h){(void)h;return CMX_UNIMPLEMENTED;}
int cdo_reset(CDOnce *o){(void)o;return CMX_UNIMPLEMENTED;}
int cdo_close(CDOnce *o){(void)o;return CMX_UNIMPLEMENTED;}
int cdo_destroy(CDOnce *o){(void)o;return CMX_UNIMPLEMENTED;}
