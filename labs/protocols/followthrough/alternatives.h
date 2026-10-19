#ifndef CMASTERY_PROTOCOL_ALTERNATIVES_H
#define CMASTERY_PROTOCOL_ALTERNATIVES_H
#include "protocols.h"
typedef struct{PTBuilder value;} PAEmpty;
typedef struct{PTBuilder value;} PAAddress;
typedef struct{PTBuilder value;} PALength;
typedef struct{PTBuilder value;} PATimeout;
int pa_address(PAEmpty,unsigned,PAAddress*);
int pa_length(PAAddress,unsigned,PALength*);
int pa_timeout(PALength,unsigned,PATimeout*);
int pa_finish(PATimeout,PTConfig*);
int pa_thread_sum(PTThread*,unsigned,uint32_t,uint32_t*);
int pa_session_pair(unsigned,unsigned*);
#endif
