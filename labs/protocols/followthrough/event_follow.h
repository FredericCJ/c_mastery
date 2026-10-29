#ifndef EVENT_FOLLOW_H
#define EVENT_FOLLOW_H
#include "protocols.h"
enum {PF_GUARDED=6,PF_LOCAL=7};
struct pf_hsm {struct pt_machine machine;unsigned events[4],head,count,in_step;};
typedef int (*pf_entry_fn)(void *,unsigned);
int pf_post(struct pf_hsm *,unsigned);
int pf_dispatch(struct pf_hsm *,unsigned,unsigned guard,unsigned local,pf_entry_fn,void *,unsigned char *,size_t,size_t *);
struct pf_lock {void *context;void (*enter)(void *);void (*leave)(void *);};
struct pf_active {struct pf_hsm hsm;struct pf_lock lock;uint32_t discarded;};
int pf_active_post(struct pf_active *,unsigned);
int pf_active_step(struct pf_active *,unsigned char *,size_t,size_t *);
int pf_active_discard(struct pf_active *,unsigned *);
struct pf_node {unsigned kind,left,right;};
struct pf_occurrence {unsigned node,left,right,cursor;};
struct pf_reactive {struct pf_occurrence frames[16];unsigned count,active,ticking;};
typedef unsigned (*pf_leaf_fn)(void *,unsigned occurrence,unsigned node);
typedef void (*pf_abort_fn)(void *,unsigned occurrence,unsigned node);
int pf_reactive_init(struct pf_reactive *,const struct pf_node *,size_t);
int pf_reactive_tick(struct pf_reactive *,const struct pf_node *,size_t,unsigned,pf_leaf_fn,pf_abort_fn,void *,unsigned *);
int pf_reactive_cancel(struct pf_reactive *,pf_abort_fn,void *);
#endif
