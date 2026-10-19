#include "query_follow.h"
int cf_volcano_open(CFVolcano*a,const CORow*b,size_t c){(void)a;(void)b;(void)c;return CO_UNIMPLEMENTED;}
int cf_volcano_next(CFVolcano*a,unsigned*b,uint32_t*c){(void)a;(void)b;(void)c;return CO_UNIMPLEMENTED;}
int cf_volcano_close(CFVolcano*a){(void)a;return CO_UNIMPLEMENTED;}
int cf_batch_open(CFBatch*a,const CORow*b,size_t c){(void)a;(void)b;(void)c;return CO_UNIMPLEMENTED;}
int cf_batch_poll(CFBatch*a,size_t b,unsigned c,unsigned d,unsigned*e,uint32_t*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return CO_UNIMPLEMENTED;}
int cf_batch_cancel(CFBatch*a){(void)a;return CO_UNIMPLEMENTED;}
int cf_order(const CORow*a,size_t b,unsigned c,CORow*d,size_t e,size_t*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return CO_UNIMPLEMENTED;}
