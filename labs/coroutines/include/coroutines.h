#ifndef CMASTERY_COROUTINES_H
#define CMASTERY_COROUTINES_H
#include <stddef.h>
#include <stdint.h>
enum{CO_OK=0,CO_INVALID=1,CO_RANGE=2,CO_STATE=3,CO_PENDING=4,CO_IO=5,CO_NO_MATCH=6,CO_DONE=7,CO_REQUEST=8,CO_UNIMPLEMENTED=9};
typedef struct{uint32_t current,end,step;unsigned done;} CORange;
typedef struct{CORange source;size_t remaining,pulls;} COFlow;
int co_next(CORange*,uint32_t*);
int co_flow_next(COFlow*,uint32_t*);
typedef int (*COFold)(void*,uint32_t); /* 0 continue,1 typed break,other error */
int co_fold(COFlow*,COFold,void*,size_t *accepted);
typedef struct{unsigned key;int32_t value;} CORow;
typedef struct{const CORow *rows;size_t count,index;unsigned closed;} COQuery;
int co_query_open(COQuery*,const CORow*,size_t);
int co_query_next(COQuery*,unsigned *key,uint32_t *sum);
int co_query_close(COQuery*);
typedef void *(*COAlloc)(void*,size_t);
typedef void (*COFree)(void*,void*);
typedef struct{void *context;COAlloc allocate;COFree release;} COAllocator;
typedef struct{COAllocator allocator;uint32_t a,b;unsigned index,count;} COGenerator;
int co_generator_create(COAllocator,unsigned,COGenerator**);
int co_generator_next(COGenerator*,uint32_t*);
int co_generator_destroy(COGenerator**);
typedef struct{unsigned n,depth,phase,stack[12];uint32_t value;} COTrampoline;
int co_trampoline_init(COTrampoline*,unsigned);
int co_trampoline_step(COTrampoline*,unsigned budget,uint32_t*);
int co_trampoline_cancel(COTrampoline*);
enum{CO_LITERAL=0,CO_SEQUENCE=1,CO_CHOICE=2,CO_MANY=3,CO_EMPTY=4};
typedef struct{unsigned kind,left,right;unsigned char literal;} COParser;
int co_parse(const COParser*,size_t,const unsigned char*,size_t,size_t,unsigned,size_t*,unsigned*);
typedef struct{unsigned state;size_t end;} COMemo;
int co_packrat(const unsigned char*,size_t,COMemo*,size_t,size_t*,unsigned*);
enum{CO_READ_DATA=0,CO_READ_AGAIN=1,CO_READ_EOF=2,CO_READ_ERROR=3};
typedef struct{unsigned status;size_t count;} COReadResult;
typedef COReadResult (*CORead)(void*,unsigned char*,size_t);
typedef int (*COArm)(void*,uint32_t);
typedef void (*CODisarm)(void*,uint32_t);
typedef struct{void *context;CORead read;COArm arm;CODisarm disarm;} COAsyncPort;
typedef struct{unsigned char data[8];size_t length,wanted;unsigned state,registered,ready;uint32_t generation;} COAsync;
int co_async_init(COAsync*,size_t,uint32_t);
int co_async_poll(COAsync*,COAsyncPort,unsigned char*,size_t*);
int co_async_notify(COAsync*,uint32_t);
int co_async_cancel(COAsync*,COAsyncPort);
typedef struct COEffect{unsigned pc;uint32_t generation,first;} COEffect;
typedef struct{COEffect *owner;uint32_t generation;unsigned question;} COContinuation;
int co_effect_start(COEffect*,COContinuation*);
int co_effect_resume(COContinuation*,uint32_t,COContinuation*,uint32_t*);
int co_effect_abort(COContinuation*);
#endif
