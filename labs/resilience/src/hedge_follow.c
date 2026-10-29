#include "hedge.h"
int rzh_init(struct rzh_arbiter *a,void *c,uint64_t (*clock)(void *),int (*decode)(void *,const unsigned char *,size_t,unsigned *)){(void)a;(void)c;(void)clock;(void)decode;return RZ_INVALID;}
int rzh_begin(struct rzh_arbiter *a,struct rzf_scope *s,uint64_t d,unsigned c,unsigned i,uint64_t f,struct rzh_ticket *o){(void)a;(void)s;(void)d;(void)c;(void)i;(void)f;(void)o;return RZ_INVALID;}
int rzh_start(struct rzh_arbiter *a,struct rzh_ticket *o){(void)a;(void)o;return RZ_INVALID;}
int rzh_effect(struct rzh_arbiter *a,struct rzh_ticket t,int d,int *o,bool *b){(void)a;(void)t;(void)d;(void)o;(void)b;return RZ_INVALID;}
int rzh_complete(struct rzh_arbiter *a,struct rzh_ticket t,struct rzh_blob *b){(void)a;(void)t;(void)b;return RZ_INVALID;}
int rzh_cancelled(struct rzh_arbiter *a,struct rzh_ticket t,bool *o){(void)a;(void)t;(void)o;return RZ_INVALID;}
int rzh_take(struct rzh_arbiter *a,struct rzh_result *o){(void)a;(void)o;return RZ_INVALID;}
int rzh_abandon(struct rzh_arbiter *a){(void)a;return RZ_INVALID;}
int rzh_snapshot(struct rzh_arbiter *a,struct rzh_snapshot *o){(void)a;(void)o;return RZ_INVALID;}
int rzh_quiescent(struct rzh_arbiter *a){(void)a;return RZ_INVALID;}
int rzh_destroy(struct rzh_arbiter *a){(void)a;return RZ_INVALID;}
