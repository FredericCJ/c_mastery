#include "outbox.h"
int rzs_open(RzjJournal *j,RzjIO io,MsgHub *h,RzsOutbox *o){(void)j;(void)io;(void)h;(void)o;return RZ_INVALID;}
int rzs_apply(RzsOutbox *s,uint64_t n,RzjRequest r,int *o,bool *d){(void)s;(void)n;(void)r;(void)o;(void)d;return RZ_INVALID;}
int rzs_rotate(RzsOutbox *s,uint64_t n,uint64_t *o){(void)s;(void)n;(void)o;return RZ_INVALID;}
int rzs_forward(RzsOutbox *s){(void)s;return RZ_INVALID;}
int rzs_deliver(RzsOutbox *s,RzsEvent *o){(void)s;(void)o;return RZ_INVALID;}
int rzs_ack(RzsOutbox *s,unsigned n){(void)s;(void)n;return RZ_INVALID;}
