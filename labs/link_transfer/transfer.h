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
struct lt_layer {
  uint32_t header, alignment, tag;
};
int lt_sizes(uint32_t, const struct lt_layer *, size_t, uint32_t *);
/* Retained immutable payload; copied two-byte prefix; accepted prefixes are
 * copied by the downstream service and cannot be rolled back. */
struct lt_borrow {
  const unsigned char *bytes;
  size_t length, used, offered;
  unsigned phase;
  unsigned char prefix[2];
};
int lt_begin(struct lt_borrow *, const unsigned char *, size_t);
int lt_offer(struct lt_borrow *, const unsigned char **, size_t *);
int lt_accept(struct lt_borrow *, size_t);
int lt_cancel(struct lt_borrow *, unsigned reset_confirmed);
struct lt_bridge {
  struct ed_bridge table[2];
  uint32_t now;
};
int lt_bridge(struct lt_bridge *, unsigned vid, const uint8_t *, const uint8_t *,
              unsigned, uint32_t, unsigned *);
int lt_dp83848(const struct ed_bus *, uint32_t, unsigned, unsigned, uint32_t, struct ed_phy *);
/* Changed lower service: length-delimited original fragment bytes, not the
 * original RS485 frame. Whole-fragment acceptance still commits E7 timing. */
struct lt_partial {
  struct e7_sender *sender;
  struct lt_borrow lower;
  unsigned char bytes[32];
  uint32_t token, last, deadline;
  unsigned active, poison;
};
int lt_partial_begin(struct lt_partial *, struct e7_sender *, uint32_t);
int lt_partial_progress(struct lt_partial *, size_t, uint32_t);
int lt_partial_abort(struct lt_partial *, unsigned reset_confirmed, uint32_t);
#endif
