#ifndef CHECKPOINT_TRANSFER_ADAPTERS_H
#define CHECKPOINT_TRANSFER_ADAPTERS_H
#include "ownership.h"
#include "errors.h"
#include "checkpoint_writer.h"
typedef struct { OwOwner owner;uint64_t cookie;unsigned closing,requests; } CPXOwner;
OwStatus cpx_owner_close(CPXOwner *,int (*unregister)(void *,uint64_t),void *);
OwStatus cpx_owner_ack(CPXOwner *,uint64_t,int);
typedef struct { er_progress progress;CPWError error;unsigned callbacks; } CPXProgress;
er_code cpx_progress_step(CPXProgress *,const unsigned char *,size_t,CPWWrite,void *);
void cpx_progress_cleanup(CPXProgress *,int);
#endif
