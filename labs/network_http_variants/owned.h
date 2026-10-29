#ifndef NV_OWNED_H
#define NV_OWNED_H
#include "variant.h"
#include "flow.h"
#include <pthread.h>
struct nv_owned { pthread_mutex_t mutex;struct hf_pool core;unsigned closing[4]; };
struct nv_lease { struct nv_owned *owner;struct hf_handle handle;int fd;unsigned active; };
/* Initialize fresh storage once; controls/active leases are noncopied.
   Successful admit transfers an owned Linux descriptor. Failure retains it.
   All threads must join before destroy; successful destroy needs no live fd. */
int nv_owned_init(struct nv_owned *);
int nv_owned_admit(struct nv_owned *,int,struct hf_handle *);
int nv_owned_borrow(struct nv_owned *,struct hf_handle,struct nv_lease *);
int nv_owned_close(struct nv_owned *,struct hf_handle);
int nv_owned_release(struct nv_lease *);
int nv_owned_destroy(struct nv_owned *);
#endif
