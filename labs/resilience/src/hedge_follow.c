#include "hedge.h"
int rzh_init(RzhArbiter *a,void *c,uint64_t (*clock)(void *),int (*decode)(void *,const unsigned char *,size_t,unsigned *)){(void)a;(void)c;(void)clock;(void)decode;return RZ_INVALID;}
int rzh_begin(RzhArbiter *a,RzfScope *s,uint64_t d,unsigned c,unsigned i,uint64_t f,RzhTicket *o){(void)a;(void)s;(void)d;(void)c;(void)i;(void)f;(void)o;return RZ_INVALID;}
int rzh_start(RzhArbiter *a,RzhTicket *o){(void)a;(void)o;return RZ_INVALID;}
int rzh_effect(RzhArbiter *a,RzhTicket t,int d,int *o,bool *b){(void)a;(void)t;(void)d;(void)o;(void)b;return RZ_INVALID;}
int rzh_complete(RzhArbiter *a,RzhTicket t,RzhBlob *b){(void)a;(void)t;(void)b;return RZ_INVALID;}
int rzh_cancelled(RzhArbiter *a,RzhTicket t,bool *o){(void)a;(void)t;(void)o;return RZ_INVALID;}
int rzh_take(RzhArbiter *a,RzhResult *o){(void)a;(void)o;return RZ_INVALID;}
int rzh_abandon(RzhArbiter *a){(void)a;return RZ_INVALID;}
int rzh_snapshot(RzhArbiter *a,RzhSnapshot *o){(void)a;(void)o;return RZ_INVALID;}
int rzh_quiescent(RzhArbiter *a){(void)a;return RZ_INVALID;}
int rzh_destroy(RzhArbiter *a){(void)a;return RZ_INVALID;}
