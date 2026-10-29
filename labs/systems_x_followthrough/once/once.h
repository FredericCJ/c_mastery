#ifndef SYSTEMS_X_ONCE_H
#define SYSTEMS_X_ONCE_H
#include "bridge.h"
struct sx_journal {unsigned id[3],tag[3],count,effects;};
struct sx_once {
 struct mb_once once;struct sx_journal *journal;unsigned id,tag,owner,stage,attempts,publications;
 unsigned table[8],checksum;
};
int sx_once_init(struct sx_once *,struct sx_journal *,unsigned id,unsigned tag,int retry);
int sx_once_enter(struct sx_once *,unsigned participant,unsigned table[8],unsigned *checksum);
int sx_once_step(struct sx_once *,unsigned participant);
int sx_once_cancel(struct sx_once *,unsigned participant);
#endif
