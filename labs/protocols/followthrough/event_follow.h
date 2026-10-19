#ifndef EVENT_FOLLOW_H
#define EVENT_FOLLOW_H
#include "protocols.h"
enum {PF_GUARDED=6,PF_LOCAL=7};
typedef struct {PTMachine machine;unsigned events[4],head,count,in_step;} PFHsm;
typedef int (*PFEntry)(void *,unsigned);
int pf_post(PFHsm *,unsigned);
int pf_dispatch(PFHsm *,unsigned,unsigned guard,unsigned local,PFEntry,void *,unsigned char *,size_t,size_t *);
typedef struct {void *context;void (*enter)(void *);void (*leave)(void *);} PFLock;
typedef struct {PFHsm hsm;PFLock lock;uint32_t discarded;} PFActive;
int pf_active_post(PFActive *,unsigned);
int pf_active_step(PFActive *,unsigned char *,size_t,size_t *);
int pf_active_discard(PFActive *,unsigned *);
typedef struct {unsigned kind,left,right;} PFNode;
typedef struct {unsigned node,left,right,cursor;} PFOccurrence;
typedef struct {PFOccurrence frames[16];unsigned count,active,ticking;} PFReactive;
typedef unsigned (*PFLeaf)(void *,unsigned occurrence,unsigned node);
typedef void (*PFAbort)(void *,unsigned occurrence,unsigned node);
int pf_reactive_init(PFReactive *,const PFNode *,size_t);
int pf_reactive_tick(PFReactive *,const PFNode *,size_t,unsigned,PFLeaf,PFAbort,void *,unsigned *);
int pf_reactive_cancel(PFReactive *,PFAbort,void *);
#endif
