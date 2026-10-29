#ifndef SYSTEMS_X_LIST_H
#define SYSTEMS_X_LIST_H
#include "bridge.h"
#include <limits.h>
enum {SX_HAZARD,SX_EPOCH};
struct sx_participant {unsigned slot,generation;};
struct sx_observed {unsigned section,from,target;};
struct sx_protected {unsigned section,key;};
struct sx_node {unsigned state,generation,value,next;};
struct sx_reader {unsigned registered,generation,active,section,hazard[2];};
struct sx_list {
 atomic_flag gate;unsigned mode,head,tail;struct sx_node node[3];struct sx_reader reader[2];struct mb_epoch epoch;
};
int sx_list_init(struct sx_list *,unsigned mode);
int sx_register(struct sx_list *,struct sx_participant *);
int sx_depart(struct sx_list *,struct sx_participant);
int sx_enter(struct sx_list *,struct sx_participant);
int sx_leave(struct sx_list *,struct sx_participant);
int sx_append(struct sx_list *,unsigned value);
int sx_remove(struct sx_list *,unsigned *value);
int sx_observe(struct sx_list *,struct sx_participant,unsigned from,struct sx_observed *);
int sx_protect(struct sx_list *,struct sx_participant,unsigned hazard_slot,struct sx_observed,struct sx_protected *);
int sx_read(struct sx_list *,struct sx_participant,struct sx_protected,unsigned *);
int sx_clear(struct sx_list *,struct sx_participant,unsigned hazard_slot);
int sx_collect(struct sx_list *,unsigned *);
int sx_advance(struct sx_list *);
#endif
