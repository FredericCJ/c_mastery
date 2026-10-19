#ifndef SYSTEMS_X_TRANSFERS_H
#define SYSTEMS_X_TRANSFERS_H
#include "b04.h"
#include "b05.h"
#include "systems.h"
typedef struct {B5Page pages[4];unsigned version[4];} SxMap;
typedef void (*SxBetween)(SxMap *,void *);
int sx_remap(SxMap *,unsigned page,unsigned present,unsigned user,unsigned writable,unsigned char fill);
int sx_copy(SxMap *,size_t,const unsigned char *,size_t,SxBetween,void *);
typedef struct {size_t copied;int status;} SxCopyResult;
int sx_partial(SxMap *,size_t,const unsigned char *,size_t,SxCopyResult *);
typedef struct {unsigned slot,generation;uint32_t identity;} SxAlias;
typedef struct {B4Ledger ledger;unsigned generation[4],owner[4],allocator[4];size_t attributed[2];} SxLedger;
int sx_allocate(SxLedger *,uint32_t,size_t,unsigned owner,unsigned allocator,SxAlias *);
int sx_resize(SxLedger *,SxAlias,unsigned actor,size_t,int succeeded,SxAlias *);
int sx_transfer(SxLedger *,SxAlias,unsigned old_owner,unsigned new_owner);
int sx_release(SxLedger *,SxAlias,unsigned actor,unsigned allocator);
int sx_lookup(const SxLedger *,SxAlias,unsigned actor,size_t *);
typedef struct {SShootdown request;unsigned online,draining,quiescent;uint32_t incarnation[8],pending_incarnation[8];} SxHotplug;
int sx_online(SxHotplug *,unsigned cpu);
int sx_begin(SxHotplug *,unsigned targets);
int sx_offline_request(SxHotplug *,unsigned cpu,uint32_t incarnation);
int sx_offline_quiescent(SxHotplug *,unsigned cpu,uint32_t incarnation);
int sx_ack(SxHotplug *,unsigned cpu,uint32_t incarnation,uint32_t generation);
int sx_reclaim(SxHotplug *);
#endif
