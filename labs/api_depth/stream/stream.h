#ifndef CMASTERY_API_STREAM_H
#define CMASTERY_API_STREAM_H
#include <stddef.h>
#include <stdint.h>
enum{SF_OK,SF_INVALID,SF_RANGE,SF_STATE,SF_PENDING,SF_IO,SF_DONE,SF_CANCELLED,SF_UNIMPLEMENTED};
typedef ptrdiff_t(*SFWrite)(void*,const unsigned char*,size_t);
typedef struct{unsigned char bytes[66];size_t length,offset;uint32_t generation;unsigned state;} SFStream;
typedef struct{SFStream*owner;uint32_t generation;} SFHandle;
void sf_init(SFStream*);
int sf_start(SFStream*,const unsigned char*,size_t,SFHandle*);
int sf_pump(SFHandle,SFWrite,void*,unsigned,size_t*);
int sf_cancel(SFHandle);
#endif
