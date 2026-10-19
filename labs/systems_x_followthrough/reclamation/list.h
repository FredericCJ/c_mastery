#ifndef SYSTEMS_X_LIST_H
#define SYSTEMS_X_LIST_H
#include "bridge.h"
#include <limits.h>
enum {SX_HAZARD,SX_EPOCH};
typedef struct {unsigned slot,generation;} SxParticipant;
typedef struct {unsigned section,from,target;} SxObserved;
typedef struct {unsigned section,key;} SxProtected;
typedef struct {unsigned state,generation,value,next;} SxNode;
typedef struct {unsigned registered,generation,active,section,hazard[2];} SxReader;
typedef struct {
 atomic_flag gate;unsigned mode,head,tail;SxNode node[3];SxReader reader[2];MBEpoch epoch;
} SxList;
int sx_list_init(SxList *,unsigned mode);
int sx_register(SxList *,SxParticipant *);
int sx_depart(SxList *,SxParticipant);
int sx_enter(SxList *,SxParticipant);
int sx_leave(SxList *,SxParticipant);
int sx_append(SxList *,unsigned value);
int sx_remove(SxList *,unsigned *value);
int sx_observe(SxList *,SxParticipant,unsigned from,SxObserved *);
int sx_protect(SxList *,SxParticipant,unsigned hazard_slot,SxObserved,SxProtected *);
int sx_read(SxList *,SxParticipant,SxProtected,unsigned *);
int sx_clear(SxList *,SxParticipant,unsigned hazard_slot);
int sx_collect(SxList *,unsigned *);
int sx_advance(SxList *);
#endif
