#ifndef PERIPHERAL_LAYER_POSTED_H
#define PERIPHERAL_LAYER_POSTED_H
#include "mmio.h"
#include "service.h"
typedef struct {
  PmModel device;
  PmAccess pending[8];
  unsigned head, count, immediate, delivered;
} PlPosted;
int pl_post_write(PlPosted *, unsigned, unsigned, uint32_t);
int pl_post_read(PlPosted *, unsigned, unsigned, uint32_t *);
int pl_flush(PlPosted *);
typedef struct {
  unsigned phase, left;
  uint8_t byte;
} PlExchange;
int pl_exchange_begin(PlExchange *, PlPosted *, uint8_t, unsigned);
int pl_exchange_step(PlExchange *, PlPosted *, uint8_t *);
#endif
