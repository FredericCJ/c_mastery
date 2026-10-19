#include "build_cache.h"
int mpb_plan(const MpbNode *n,size_t c,unsigned f,MpbPlan *o){(void)n;(void)c;(void)f;(void)o;return MPB_INVALID;}
int mpb_match(const MpbEntry *a,const MpbEntry *b){(void)a;(void)b;return 0;}
int mpb_encode(const MpbEntry *e,unsigned i,unsigned char *b,size_t c,size_t *u){(void)e;(void)i;(void)b;(void)c;(void)u;return MPB_INVALID;}
int mpb_begin(MpbSchedule *s,const MpbPlan *p){(void)s;(void)p;return MPB_INVALID;}
int mpb_claim(MpbSchedule *s,MpbTicket *t){(void)s;(void)t;return MPB_INVALID;}
int mpb_finish(MpbSchedule *s,MpbTicket t,const MpbEntry *e){(void)s;(void)t;(void)e;return MPB_INVALID;}
int mpb_publish(const MpbSchedule *s,MpbPlan *o){(void)s;(void)o;return MPB_INVALID;}
