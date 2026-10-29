#ifndef CHECKPOINT_CHANNEL_H
#define CHECKPOINT_CHANNEL_H
#include "checkpoint_memory.h"
#include "checkpoint_writer.h"
struct cpw_channel { struct cpm_pool pool;struct cpm_ring queue;struct cpw_transfer transfer;struct cpm_handle active;unsigned char byte;int has_active; };
void cpw_channel_init(struct cpw_channel *);
enum cpm_status cpw_offer(struct cpw_channel *,const int *,size_t);
enum cpw_status cpw_channel_step(struct cpw_channel *,cpw_write_fn,void *);
#endif
