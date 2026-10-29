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
typedef void (*mb_cleanup_fn)(void *);
struct mb_region {
  _Alignas(max_align_t) unsigned char bytes[32];
  size_t cursor, payload, high, count;
  unsigned generation;
  bool busy;
  mb_cleanup_fn cleanup[4];
  void *context[4];
};
struct mb_span {
  const struct mb_region *owner;
  unsigned generation;
  size_t offset, length;
};
void mb_region_init(struct mb_region *);
int mb_region_alloc(struct mb_region *, size_t, size_t, struct mb_span *);
int mb_region_cleanup(struct mb_region *, mb_cleanup_fn, void *);
int mb_region_reset(struct mb_region *);
int mb_region_read(const struct mb_region *, struct mb_span, size_t, unsigned char *);
struct mb_once {
  atomic_uint state;
  bool retry;
  unsigned values[2];
};
void mb_once_init(struct mb_once *, bool);
int mb_once_enter(struct mb_once *, unsigned, unsigned *);
int mb_once_finish(struct mb_once *, unsigned, bool, unsigned, unsigned);
struct mb_hazards {
  atomic_uint source, hazard[2];
  unsigned generation[3], state[3], values[3];
};
void mb_hp_init(struct mb_hazards *);
int mb_hp_publish(struct mb_hazards *, unsigned);
unsigned mb_hp_observe(const struct mb_hazards *);
int mb_hp_protect(struct mb_hazards *, unsigned, unsigned, unsigned *);
void mb_hp_clear(struct mb_hazards *, unsigned);
int mb_hp_retire(struct mb_hazards *);
unsigned mb_hp_collect(struct mb_hazards *);
struct mb_epoch {
  unsigned epoch, reader_epoch[2], retired_at[3];
  bool active[2], retired[3], reclaimed[3];
};
void mb_epoch_init(struct mb_epoch *);
int mb_epoch_enter(struct mb_epoch *, unsigned);
int mb_epoch_leave(struct mb_epoch *, unsigned);
int mb_epoch_retire(struct mb_epoch *, unsigned);
int mb_epoch_advance(struct mb_epoch *);
unsigned mb_epoch_collect(struct mb_epoch *);
#endif
