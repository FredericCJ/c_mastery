#include "board.h"
#include <limits.h>
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
int ar_service(int32_t milli, int32_t threshold, struct ar_indicator i) {
  if (!i.set)
    return AR_INVALID;
  return i.set(i.context, milli >= threshold);
}
int ar_centimilli(int32_t centi, int32_t *out) {
  if (!out)
    return AR_INVALID;
  if (centi > INT32_MAX / 10 || centi < INT32_MIN / 10)
    return AR_RANGE;
  *out = centi * 10;
  return AR_OK;
}
static int valid(struct ar_board *b) {
  return b && b->read && b->write && b->pin < 16 && b->active_low < 2;
}
int ar_board_init(struct ar_board *b) {
  if (!valid(b))
    return AR_INVALID;
  uint32_t bit = 1u << b->pin, mask = 3u << (2 * b->pin);
  b->write(b->context, b->gpio + 4, b->read(b->context, b->gpio + 4) & ~bit);
  b->write(b->context, b->gpio + 8, b->read(b->context, b->gpio + 8) & ~mask);
  b->write(b->context, b->gpio + 12, b->read(b->context, b->gpio + 12) & ~mask);
  b->write(b->context, b->gpio,
           (b->read(b->context, b->gpio) & ~mask) | (1u << (2 * b->pin)));
  return AR_OK;
}
int ar_board_set(void *ctx, unsigned on) {
  struct ar_board *b = ctx;
  if (!valid(b) || on > 1)
    return AR_INVALID;
  unsigned high = on ^ (AR_MUTANT == 7 ? 0 : b->active_low);
  b->write(b->context, b->gpio + 24, (1u << b->pin) << (high ? 0 : 16));
  return AR_OK;
}
int ar_clear_pending(struct ar_board *b, uintptr_t address, uint32_t mask) {
  if (!valid(b) || !mask || (mask & ~0xffffu))
    return AR_INVALID;
  b->write(b->context, address, mask);
  return AR_OK;
}
