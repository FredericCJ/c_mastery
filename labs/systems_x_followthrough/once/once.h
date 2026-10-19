#ifndef SYSTEMS_X_ONCE_H
#define SYSTEMS_X_ONCE_H
#include "bridge.h"
typedef struct {unsigned id[3],tag[3],count,effects;} SxJournal;
typedef struct {
 MBOnce once;SxJournal *journal;unsigned id,tag,owner,stage,attempts,publications;
 unsigned table[8],checksum;
} SxOnce;
int sx_once_init(SxOnce *,SxJournal *,unsigned id,unsigned tag,int retry);
int sx_once_enter(SxOnce *,unsigned participant,unsigned table[8],unsigned *checksum);
int sx_once_step(SxOnce *,unsigned participant);
int sx_once_cancel(SxOnce *,unsigned participant);
#endif
