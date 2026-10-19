#ifndef CHECKPOINT_CHANNEL_H
#define CHECKPOINT_CHANNEL_H
#include "checkpoint_memory.h"
#include "checkpoint_writer.h"
typedef struct { CPMPool pool;CPMRing queue;CPWTransfer transfer;CPMHandle active;unsigned char byte;int has_active; } CPWChannel;
void cpw_channel_init(CPWChannel *);
CPMStatus cpw_offer(CPWChannel *,const int *,size_t);
CPWStatus cpw_channel_step(CPWChannel *,CPWWrite,void *);
#endif
