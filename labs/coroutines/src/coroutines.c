#include "coroutines.h"
int co_next(struct co_range *r,uint32_t *out){(void)r;(void)out;return CO_UNIMPLEMENTED;}
int co_flow_next(struct co_flow *f,uint32_t *out){(void)f;(void)out;return CO_UNIMPLEMENTED;}
int co_fold(struct co_flow *f,co_fold_fn fold,void *ctx,size_t *accepted){(void)f;(void)fold;(void)ctx;(void)accepted;return CO_UNIMPLEMENTED;}
int co_query_open(struct co_query *q,const struct co_row *rows,size_t n){(void)q;(void)rows;(void)n;return CO_UNIMPLEMENTED;}
int co_query_next(struct co_query *q,unsigned *key,uint32_t *sum){(void)q;(void)key;(void)sum;return CO_UNIMPLEMENTED;}
int co_query_close(struct co_query *q){(void)q;return CO_UNIMPLEMENTED;}
int co_generator_create(struct co_allocator a,unsigned count,struct co_generator **out){(void)a;(void)count;(void)out;return CO_UNIMPLEMENTED;}
int co_generator_next(struct co_generator *p,uint32_t *out){(void)p;(void)out;return CO_UNIMPLEMENTED;}
int co_generator_destroy(struct co_generator **p){(void)p;return CO_UNIMPLEMENTED;}
int co_trampoline_init(struct co_trampoline *p,unsigned n){(void)p;(void)n;return CO_UNIMPLEMENTED;}
int co_trampoline_step(struct co_trampoline *p,unsigned budget,uint32_t *out){(void)p;(void)budget;(void)out;return CO_UNIMPLEMENTED;}
int co_trampoline_cancel(struct co_trampoline *p){(void)p;return CO_UNIMPLEMENTED;}
int co_parse(const struct co_parser *p,size_t pn,const unsigned char *s,size_t n,size_t start,unsigned budget,size_t *end,unsigned *steps){(void)p;(void)pn;(void)s;(void)n;(void)start;(void)budget;(void)end;(void)steps;return CO_UNIMPLEMENTED;}
int co_packrat(const unsigned char *s,size_t n,struct co_memo *memo,size_t cap,size_t *end,unsigned *calls){(void)s;(void)n;(void)memo;(void)cap;(void)end;(void)calls;return CO_UNIMPLEMENTED;}
int co_async_init(struct co_async *p,size_t wanted,uint32_t generation){(void)p;(void)wanted;(void)generation;return CO_UNIMPLEMENTED;}
int co_async_poll(struct co_async *p,struct co_async_port port,unsigned char *out,size_t *n){(void)p;(void)port;(void)out;(void)n;return CO_UNIMPLEMENTED;}
int co_async_notify(struct co_async *p,uint32_t gen){(void)p;(void)gen;return CO_UNIMPLEMENTED;}
int co_async_cancel(struct co_async *p,struct co_async_port port){(void)p;(void)port;return CO_UNIMPLEMENTED;}
int co_effect_start(struct co_effect *p,struct co_continuation *out){(void)p;(void)out;return CO_UNIMPLEMENTED;}
int co_effect_resume(struct co_continuation *k,uint32_t value,struct co_continuation *next,uint32_t *out){(void)k;(void)value;(void)next;(void)out;return CO_UNIMPLEMENTED;}
int co_effect_abort(struct co_continuation *k){(void)k;return CO_UNIMPLEMENTED;}
