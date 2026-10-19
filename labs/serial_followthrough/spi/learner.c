#include "scheduler.h"
int ss_open(SsScheduler *a,PsIo b,const PuClock *c,unsigned d,unsigned e){(void)a;(void)b;(void)c;(void)d;(void)e;return PS_UNIMPLEMENTED;}
int ss_enqueue(SsScheduler *a,uint32_t b,const uint8_t *c,unsigned d,unsigned e,unsigned f,unsigned g,unsigned h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return PS_UNIMPLEMENTED;}
int ss_run(SsScheduler *a,PsIo b,SsSubmit c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return PS_UNIMPLEMENTED;}
int ss_event(SsScheduler *a,PsIo b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return PS_UNIMPLEMENTED;}
int ss_cancel(SsScheduler *a,PsIo b,uint32_t c){(void)a;(void)b;(void)c;return PS_UNIMPLEMENTED;}
int ss_reset(SsScheduler *a,PsIo b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return PS_UNIMPLEMENTED;}
int ss_take(SsScheduler *a,uint32_t b,SsResult *c,uint8_t d[16]){(void)a;(void)b;(void)c;(void)d;return PS_UNIMPLEMENTED;}
