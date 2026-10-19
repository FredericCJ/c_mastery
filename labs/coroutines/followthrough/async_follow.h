#ifndef CMASTERY_ASYNC_FOLLOW_H
#define CMASTERY_ASYNC_FOLLOW_H
#include "coroutines.h"
#include <pthread.h>
typedef struct CAFrame CAFrame;
typedef struct{CAFrame*owner;uint32_t generation,serial;unsigned slot;} CATicket;
struct CAFrame{
 pthread_mutex_t mutex;COAllocator allocator;uint32_t generation,next;
 unsigned wanted,length,state,closing,ack,published,inflight;
 uint32_t tickets[4];unsigned char data[8];
};
int ca_create(COAllocator,unsigned,uint32_t,CAFrame**);
int ca_reserve(CAFrame*,CATicket*);
int ca_notify(CATicket*,const unsigned char*,size_t);
int ca_cancel(CAFrame*);
int ca_ack(CAFrame*);
int ca_poll(CAFrame*,unsigned char*,size_t*);
int ca_destroy(CAFrame**);
#endif
