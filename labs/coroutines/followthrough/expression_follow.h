#ifndef CMASTERY_EXPRESSION_FOLLOW_H
#define CMASTERY_EXPRESSION_FOLLOW_H
#include "coroutines.h"
enum{CE_LITERAL,CE_ADD,CE_MULTIPLY};
struct ce_node{unsigned kind,left,right;int32_t value;};
struct ce_continuation{unsigned node,phase;int32_t*left;};
typedef int(*ce_deliver_fn)(void*,int32_t); /* OK acknowledgement, PENDING suspension, other error */
struct ce_frame{
 const struct ce_node*nodes;size_t count;struct co_allocator allocator;
 struct ce_continuation stack[16];unsigned depth,state,have,max_depth,steps;
 int32_t result;ce_deliver_fn deliver;void*context;
};
int ce_init(struct ce_frame*,const struct ce_node*,size_t,struct co_allocator,ce_deliver_fn,void*);
int ce_step(struct ce_frame*,unsigned,int32_t*);
int ce_cancel(struct ce_frame*);
#endif
