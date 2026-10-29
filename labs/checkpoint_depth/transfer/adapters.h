#ifndef CHECKPOINT_TRANSFER_ADAPTERS_H
#define CHECKPOINT_TRANSFER_ADAPTERS_H
#include "ownership.h"
#include "errors.h"
#include "checkpoint_writer.h"
struct cpx_owner { struct ow_owner owner;uint64_t cookie;unsigned closing,requests; };
enum ow_status cpx_owner_close(struct cpx_owner *,int (*unregister)(void *,uint64_t),void *);
enum ow_status cpx_owner_ack(struct cpx_owner *,uint64_t,int);
struct cpx_progress { er_progress progress;struct cpw_error error;unsigned callbacks; };
er_code cpx_progress_step(struct cpx_progress *,const unsigned char *,size_t,cpw_write_fn,void *);
void cpx_progress_cleanup(struct cpx_progress *,int);
#endif
