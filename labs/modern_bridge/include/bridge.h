#ifndef CMASTERY_MODERN_BRIDGE_H
#define CMASTERY_MODERN_BRIDGE_H
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
enum {
  MB_OK,
  MB_INVALID,
  MB_FULL,
  MB_STALE,
  MB_BUSY,
  MB_INIT,
  MB_RECURSIVE,
  MB_FAILED,
  MB_RETRY,
  MB_EMPTY
};
typedef void (*MBCleanup)(void *);
typedef struct MBRegion {
  _Alignas(max_align_t) unsigned char bytes[32];
  size_t cursor, payload, high, count;
  unsigned generation;
  bool busy;
  MBCleanup cleanup[4];
  void *context[4];
} MBRegion;
typedef struct {
  const MBRegion *owner;
  unsigned generation;
  size_t offset, length;
} MBSpan;
void mb_region_init(MBRegion *);
int mb_region_alloc(MBRegion *, size_t, size_t, MBSpan *);
int mb_region_cleanup(MBRegion *, MBCleanup, void *);
int mb_region_reset(MBRegion *);
int mb_region_read(const MBRegion *, MBSpan, size_t, unsigned char *);
typedef struct {
  atomic_uint state;
  bool retry;
  unsigned values[2];
} MBOnce;
void mb_once_init(MBOnce *, bool);
int mb_once_enter(MBOnce *, unsigned, unsigned *);
int mb_once_finish(MBOnce *, unsigned, bool, unsigned, unsigned);
typedef struct {
  atomic_uint source, hazard[2];
  unsigned generation[3], state[3], values[3];
} MBHazards;
void mb_hp_init(MBHazards *);
int mb_hp_publish(MBHazards *, unsigned);
unsigned mb_hp_observe(const MBHazards *);
int mb_hp_protect(MBHazards *, unsigned, unsigned, unsigned *);
void mb_hp_clear(MBHazards *, unsigned);
int mb_hp_retire(MBHazards *);
unsigned mb_hp_collect(MBHazards *);
typedef struct {
  unsigned epoch, reader_epoch[2], retired_at[3];
  bool active[2], retired[3], reclaimed[3];
} MBEpoch;
void mb_epoch_init(MBEpoch *);
int mb_epoch_enter(MBEpoch *, unsigned);
int mb_epoch_leave(MBEpoch *, unsigned);
int mb_epoch_retire(MBEpoch *, unsigned);
int mb_epoch_advance(MBEpoch *);
unsigned mb_epoch_collect(MBEpoch *);
#endif
