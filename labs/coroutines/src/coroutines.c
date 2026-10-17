#include "coroutines.h"
int co_next(CORange *r,uint32_t *out){(void)r;(void)out;return CO_UNIMPLEMENTED;}
int co_flow_next(COFlow *f,uint32_t *out){(void)f;(void)out;return CO_UNIMPLEMENTED;}
int co_fold(COFlow *f,COFold fold,void *ctx,size_t *accepted){(void)f;(void)fold;(void)ctx;(void)accepted;return CO_UNIMPLEMENTED;}
int co_query_open(COQuery *q,const CORow *rows,size_t n){(void)q;(void)rows;(void)n;return CO_UNIMPLEMENTED;}
int co_query_next(COQuery *q,unsigned *key,uint32_t *sum){(void)q;(void)key;(void)sum;return CO_UNIMPLEMENTED;}
int co_query_close(COQuery *q){(void)q;return CO_UNIMPLEMENTED;}
int co_generator_create(COAllocator a,unsigned count,COGenerator **out){(void)a;(void)count;(void)out;return CO_UNIMPLEMENTED;}
int co_generator_next(COGenerator *p,uint32_t *out){(void)p;(void)out;return CO_UNIMPLEMENTED;}
int co_generator_destroy(COGenerator **p){(void)p;return CO_UNIMPLEMENTED;}
int co_trampoline_init(COTrampoline *p,unsigned n){(void)p;(void)n;return CO_UNIMPLEMENTED;}
int co_trampoline_step(COTrampoline *p,unsigned budget,uint32_t *out){(void)p;(void)budget;(void)out;return CO_UNIMPLEMENTED;}
int co_trampoline_cancel(COTrampoline *p){(void)p;return CO_UNIMPLEMENTED;}
int co_parse(const COParser *p,size_t pn,const unsigned char *s,size_t n,size_t start,unsigned budget,size_t *end,unsigned *steps){(void)p;(void)pn;(void)s;(void)n;(void)start;(void)budget;(void)end;(void)steps;return CO_UNIMPLEMENTED;}
int co_packrat(const unsigned char *s,size_t n,COMemo *memo,size_t cap,size_t *end,unsigned *calls){(void)s;(void)n;(void)memo;(void)cap;(void)end;(void)calls;return CO_UNIMPLEMENTED;}
int co_async_init(COAsync *p,size_t wanted,uint32_t generation){(void)p;(void)wanted;(void)generation;return CO_UNIMPLEMENTED;}
int co_async_poll(COAsync *p,COAsyncPort port,unsigned char *out,size_t *n){(void)p;(void)port;(void)out;(void)n;return CO_UNIMPLEMENTED;}
int co_async_notify(COAsync *p,uint32_t gen){(void)p;(void)gen;return CO_UNIMPLEMENTED;}
int co_async_cancel(COAsync *p,COAsyncPort port){(void)p;(void)port;return CO_UNIMPLEMENTED;}
int co_effect_start(COEffect *p,COContinuation *out){(void)p;(void)out;return CO_UNIMPLEMENTED;}
int co_effect_resume(COContinuation *k,uint32_t value,COContinuation *next,uint32_t *out){(void)k;(void)value;(void)next;(void)out;return CO_UNIMPLEMENTED;}
int co_effect_abort(COContinuation *k){(void)k;return CO_UNIMPLEMENTED;}
