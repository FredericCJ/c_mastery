#include "retry.h"
int rzr_reduce(uint32_t s,unsigned b,uint32_t n,uint32_t *o){(void)s;(void)b;(void)n;(void)o;return RZ_INVALID;}
int rzr_uniform(RzrRandom *r,uint32_t n,uint32_t *o){(void)r;(void)n;(void)o;return RZ_INVALID;}
int rzr_init(RzrRetry *r,RzfScope *s,RzfBucket *b,NetStack *n,unsigned m,uint64_t a,uint64_t c,bool f,uint32_t d){(void)r;(void)s;(void)b;(void)n;(void)m;(void)a;(void)c;(void)f;(void)d;return RZ_INVALID;}
int rzr_original(RzrRetry *r,uint64_t n,V4PacketSink s,void *c){(void)r;(void)n;(void)s;(void)c;return RZ_INVALID;}
int rzr_plan(RzrRetry *r,uint64_t n,uint64_t a,RzrJitter j,uint64_t *o){(void)r;(void)n;(void)a;(void)j;(void)o;return RZ_INVALID;}
int rzr_dispatch(RzrRetry *r,uint64_t n,V4PacketSink s,void *c){(void)r;(void)n;(void)s;(void)c;return RZ_INVALID;}
int rzr_ack_packet(RzrRetry *r,const void *b,size_t z,uint64_t n,NetPacket *o){(void)r;(void)b;(void)z;(void)n;(void)o;return RZ_INVALID;}
int rzr_finish(RzrRetry *r){(void)r;return RZ_INVALID;}
int rzr_abort(RzrRetry *r){(void)r;return RZ_INVALID;}
