#ifndef LT_TRANSFER_H
#define LT_TRANSFER_H
#include "com_edu7.h"
#include "edu7_depth.h"
#include "ethernet_depth.h"
enum {
  LT_OK,
  LT_INVALID,
  LT_BUSY,
  LT_AMBIGUOUS,
  LT_STALE,
  LT_FULL,
  LT_TIMEOUT,
  LT_DONE,
  LT_STUB = 99
};
int lt_order(unsigned modulus, unsigned a, unsigned b, int *order);
int lt_ack(unsigned modulus, unsigned *base, unsigned *outstanding,
           unsigned next);
typedef struct {
  uint32_t header, alignment, tag;
} LtLayer;
int lt_sizes(uint32_t, const LtLayer *, size_t, uint32_t *);
/* Retained immutable payload; copied two-byte prefix; accepted prefixes are
 * copied by the downstream service and cannot be rolled back. */
typedef struct {
  const unsigned char *bytes;
  size_t length, used, offered;
  unsigned phase;
  unsigned char prefix[2];
} LtBorrow;
int lt_begin(LtBorrow *, const unsigned char *, size_t);
int lt_offer(LtBorrow *, const unsigned char **, size_t *);
int lt_accept(LtBorrow *, size_t);
int lt_cancel(LtBorrow *, unsigned reset_confirmed);
typedef struct {
  EDBridge table[2];
  uint32_t now;
} LtBridge;
int lt_bridge(LtBridge *, unsigned vid, const uint8_t *, const uint8_t *,
              unsigned, uint32_t, unsigned *);
int lt_dp83848(const EDBus *, uint32_t, unsigned, unsigned, uint32_t, EDPhy *);
/* Changed lower service: length-delimited original fragment bytes, not the
 * original RS485 frame. Whole-fragment acceptance still commits E7 timing. */
typedef struct {
  E7Sender *sender;
  LtBorrow lower;
  unsigned char bytes[32];
  uint32_t token, last, deadline;
  unsigned active, poison;
} LtPartial;
int lt_partial_begin(LtPartial *, E7Sender *, uint32_t);
int lt_partial_progress(LtPartial *, size_t, uint32_t);
int lt_partial_abort(LtPartial *, unsigned reset_confirmed, uint32_t);
#endif
