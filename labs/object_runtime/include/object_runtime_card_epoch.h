#ifndef CMASTERY_OBJECT_RUNTIME_CARD_EPOCH_H
#define CMASTERY_OBJECT_RUNTIME_CARD_EPOCH_H
#include "object_runtime.h"
#include <pthread.h>
enum { ORCC_REV_LIMIT = 65535 };
typedef struct {
  uint32_t revision;
  uint8_t dirty;
} ORCCMeta;
typedef struct {
  _Alignas(64) ORCCMeta meta;
} ORCCPadded;
typedef struct {
  pthread_mutex_t mutex;
  uint8_t edge[8][2];
  unsigned width, padded;
  union {
    ORCCMeta packed[8];
    ORCCPadded padded[8];
  } metadata;
  int initialized;
} ORCCards;
typedef struct {
  const ORCCards *owner;
  unsigned card, count;
  uint32_t revision;
  uint8_t edge[16];
} ORCCSnapshot;
ORStatus orcc_init(ORCCards *, unsigned objects_per_card, int padded);
ORStatus orcc_store(ORCCards *, unsigned object, unsigned field,
                    unsigned target);
ORStatus orcc_prepare(ORCCards *, unsigned card, ORCCSnapshot *);
ORStatus orcc_scan_commit(ORCCards *, const ORCCSnapshot *, uint16_t *roots,
                          unsigned *scanned);
ORStatus orcc_close(ORCCards *);
#endif
