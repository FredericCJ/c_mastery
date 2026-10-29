#ifndef CMASTERY_COROUTINE_ALTERNATIVES_H
#define CMASTERY_COROUTINE_ALTERNATIVES_H
#include "query_follow.h"
struct cr_unfused{struct co_flow flow;size_t predicates,maps;};
int cr_unfused_next(struct cr_unfused*,uint32_t*);
struct cr_caller_generator{uint32_t a,b;unsigned index,count;};
int cr_caller_init(struct cr_caller_generator*,unsigned);
int cr_caller_next(struct cr_caller_generator*,uint32_t*);
typedef int(*cr_sink_fn)(void*,unsigned,uint32_t);
int cr_push(struct cf_batch*,size_t,unsigned,cr_sink_fn,void*,unsigned,size_t*);
#endif
