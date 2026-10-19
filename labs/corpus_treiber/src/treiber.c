#include "treiber.h"
/* Implement all operations from the public contract; no hidden reference import. */
TsStatus ts_init(TsStack*s,unsigned n){(void)s;(void)n;return TS_UNSUPPORTED;}
TsStatus ts_push(TsStack*s,uint32_t n){(void)s;(void)n;return TS_UNSUPPORTED;}
TsStatus ts_begin(TsStack*s,unsigned p,TsPop*v){(void)s;(void)p;(void)v;return TS_UNSUPPORTED;}
TsStatus ts_commit(TsPop*v,uint32_t*out){(void)v;(void)out;return TS_UNSUPPORTED;}
TsStatus ts_cancel(TsPop*v){(void)v;return TS_UNSUPPORTED;}
TsStatus ts_pop(TsStack*s,unsigned p,uint32_t*out){(void)s;(void)p;(void)out;return TS_UNSUPPORTED;}
TsStatus ts_scan(TsStack*s,unsigned n,TsRetired*r){(void)s;(void)n;(void)r;return TS_UNSUPPORTED;}
TsStatus ts_reclaim(TsRetired*r){(void)r;return TS_UNSUPPORTED;}
unsigned ts_collect(TsStack*s){(void)s;return 0;}
