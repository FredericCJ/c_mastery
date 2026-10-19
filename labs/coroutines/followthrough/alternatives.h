#ifndef CMASTERY_COROUTINE_ALTERNATIVES_H
#define CMASTERY_COROUTINE_ALTERNATIVES_H
#include "query_follow.h"
typedef struct{COFlow flow;size_t predicates,maps;} CRUnfused;
int cr_unfused_next(CRUnfused*,uint32_t*);
typedef struct{uint32_t a,b;unsigned index,count;} CRCallerGenerator;
int cr_caller_init(CRCallerGenerator*,unsigned);
int cr_caller_next(CRCallerGenerator*,uint32_t*);
typedef int(*CRSink)(void*,unsigned,uint32_t);
int cr_push(CFBatch*,size_t,unsigned,CRSink,void*,unsigned,size_t*);
#endif
