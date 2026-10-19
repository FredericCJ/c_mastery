#ifndef NV_OWNED_H
#define NV_OWNED_H
#include "variant.h"
#include "flow.h"
#include <pthread.h>
typedef struct { pthread_mutex_t mutex;HFPool core;unsigned closing[4]; } NVOwned;
typedef struct { NVOwned *owner;HFHandle handle;int fd;unsigned active; } NVLease;
/* Initialize fresh storage once; controls/active leases are noncopied.
   Successful admit transfers an owned Linux descriptor. Failure retains it.
   All threads must join before destroy; successful destroy needs no live fd. */
int nv_owned_init(NVOwned *);
int nv_owned_admit(NVOwned *,int,HFHandle *);
int nv_owned_borrow(NVOwned *,HFHandle,NVLease *);
int nv_owned_close(NVOwned *,HFHandle);
int nv_owned_release(NVLease *);
int nv_owned_destroy(NVOwned *);
#endif
