#ifndef CMASTERY_EXPRESSION_FOLLOW_H
#define CMASTERY_EXPRESSION_FOLLOW_H
#include "coroutines.h"
enum{CE_LITERAL,CE_ADD,CE_MULTIPLY};
typedef struct{unsigned kind,left,right;int32_t value;} CENode;
typedef struct{unsigned node,phase;int32_t*left;} CEContinuation;
typedef int(*CEDeliver)(void*,int32_t); /* OK acknowledgement, PENDING suspension, other error */
typedef struct{
 const CENode*nodes;size_t count;COAllocator allocator;
 CEContinuation stack[16];unsigned depth,state,have,max_depth,steps;
 int32_t result;CEDeliver deliver;void*context;
} CEFrame;
int ce_init(CEFrame*,const CENode*,size_t,COAllocator,CEDeliver,void*);
int ce_step(CEFrame*,unsigned,int32_t*);
int ce_cancel(CEFrame*);
#endif
