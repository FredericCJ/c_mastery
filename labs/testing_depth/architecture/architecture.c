/* Intentionally unfinished learner implementation. Do not ship assessor source. */
#include "architecture.h"
enum ta_status ta_init(struct ta_queue*q,struct ta_storage*s,enum ta_layout l,unsigned c,unsigned e){(void)q;(void)s;(void)l;(void)c;(void)e;return TA_INVALID;}
enum ta_status ta_push(struct ta_queue*q,unsigned char v){(void)q;(void)v;return TA_INVALID;}
enum ta_status ta_pop(struct ta_queue*q,unsigned char*v){(void)q;(void)v;return TA_INVALID;}
enum ta_status ta_peek(const struct ta_queue*q,unsigned char*v){(void)q;(void)v;return TA_INVALID;}
size_t ta_count(const struct ta_queue*q){(void)q;return 0;}
enum ta_status ta_batch(struct ta_queue*q,const unsigned char*p,size_t n,size_t*a){(void)q;(void)p;(void)n;(void)a;return TA_INVALID;}
int ta_invariant(const struct ta_queue*q){(void)q;return 0;}
enum ta_status ta_decode(const unsigned char*p,size_t n,unsigned char o[2]){(void)p;(void)n;(void)o;return TA_INVALID;}
enum ta_status ta_begin(struct ta_flow*f,const unsigned char*p,size_t n,enum ta_layout l,unsigned b){(void)f;(void)p;(void)n;(void)l;(void)b;return TA_INVALID;}
enum ta_status ta_step(struct ta_flow*f,ta_write_fn w,void*c){(void)f;(void)w;(void)c;return TA_INVALID;}
enum ta_status ta_cancel(struct ta_flow*f){(void)f;return TA_INVALID;}
