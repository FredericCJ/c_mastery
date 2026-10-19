#include "order.h"
void mf_order_init(MfOrder *q,uint32_t epoch,unsigned heap,unsigned reverse){(void)q;(void)epoch;(void)heap;(void)reverse;}
int mf_order_push(MfOrder *q,MfOrderEntry e){(void)q;(void)e;return 0;}
int mf_order_pop(MfOrder *q,MfOrderEntry *e){(void)q;(void)e;return 0;}
