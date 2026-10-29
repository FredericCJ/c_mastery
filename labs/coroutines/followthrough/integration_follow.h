#ifndef CMASTERY_COROUTINE_INTEGRATION_H
#define CMASTERY_COROUTINE_INTEGRATION_H
#include "coroutines.h"
#include "foundation.h"
#include "stream.h"
int ci_parse_cursor(struct f_cursor*,const struct co_parser*,size_t,unsigned,size_t*,unsigned*);
struct ci_progress{size_t accepted;unsigned reads,arms,disarms;int port_result;};
int ci_receive(const int[2],int,int,long long,struct sf_stream*,struct sf_handle*,int32_t*,struct ci_progress*);
#endif
