#ifndef CMASTERY_COROUTINE_INTEGRATION_H
#define CMASTERY_COROUTINE_INTEGRATION_H
#include "coroutines.h"
#include "foundation.h"
#include "stream.h"
int ci_parse_cursor(FCursor*,const COParser*,size_t,unsigned,size_t*,unsigned*);
typedef struct{size_t accepted;unsigned reads,arms,disarms;int port_result;} CIProgress;
int ci_receive(const int[2],int,int,long long,SFStream*,SFHandle*,int32_t*,CIProgress*);
#endif
