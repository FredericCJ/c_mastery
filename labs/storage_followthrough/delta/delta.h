#ifndef STORAGE_FOLLOW_DELTA_H
#define STORAGE_FOLLOW_DELTA_H
#include "store.h"
enum {
  SD_OK = 0,
  SD_INVALID,
  SD_STATE,
  SD_FULL,
  SD_IO,
  SD_UNCERTAIN,
  SD_CORRUPT,
  SD_UNIMPLEMENTED
};
typedef struct {
  store_io io;
  uint64_t generation;
  uint32_t sums[32], dirty, wal_mask;
  unsigned active, poisoned, full;
  uint8_t scratch[32][4096];
} SdStore;
/* SDW1 raw-page transaction format: same externally visible32-page atomic
 * contract for full and sparse policies; deliberately not CST2 or its B+tree.
 * Live serialized controls and exclusive67-page backend. Successful flush
 * persists all prior accepted pages. No callback reentry/buffer retention.
 * Output buffers disjoint from control/scratch. Constructors use fresh storage.
 */
int sd_format(store_io);
int sd_open(SdStore *, store_io, unsigned full);
int sd_begin(SdStore *);
int sd_write(SdStore *, unsigned page, size_t offset, const void *, size_t);
int sd_read(SdStore *, unsigned page, void *);
int sd_commit(SdStore *);
int sd_checkpoint(SdStore *);
void sd_abort(SdStore *);
#endif
