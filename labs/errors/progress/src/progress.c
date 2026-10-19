#include "progress.h"
void ept_init(EptStream *s){(void)s;}
unsigned ept_start(EptStream *s,const unsigned char *b,size_t n,EptHandle *h){(void)s;(void)b;(void)n;(void)h;return EPT_UNIMPLEMENTED;}
unsigned ept_step(EptHandle h,er_sink f,void *c,EptObservation *o){(void)h;(void)f;(void)c;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_cancel(EptHandle h,EptObservation *o){(void)h;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_checkpoint(EptHandle h,EptFlush f,void *c,EptObservation *o){(void)h;(void)f;(void)c;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_reconcile(EptHandle h,size_t n,uint32_t e,unsigned q,EptObservation *o){(void)h;(void)n;(void)e;(void)q;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_observe(EptHandle h,EptObservation *o){(void)h;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_release(EptHandle h,unsigned q,unsigned a){(void)h;(void)q;(void)a;return EPT_UNIMPLEMENTED;}
