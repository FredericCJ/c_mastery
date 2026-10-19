#include "borrow.h"
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
int ar_borrow(ArBorrow *b, const unsigned char *p, size_t n) {
  if (!b || !p || !n || n > 8)
    return AR_INVALID;
  if (b->active)
    return AR_STATE;
  *b = (ArBorrow){p, n, 0, 1, 0};
  return AR_OK;
}
int ar_borrow_chunk(ArBorrow *b, size_t budget, unsigned char *out,
                    size_t *used) {
  if (!b || !out || !used || !budget)
    return AR_INVALID;
  if (!b->active || b->cancel)
    return AR_STATE;
  size_t n = b->length - b->offset;
  if (n > budget)
    n = budget;
  for (size_t i = 0; i < n; ++i)
    out[i] = b->data[b->offset + i];
  b->offset += n;
  *used = n;
  if (b->offset == b->length) {
    b->active = 0;
    b->data = 0;
    return AR_OK;
  }
  return AR_PENDING;
}
int ar_borrow_cancel(ArBorrow *b) {
  if (!b || !b->active)
    return AR_STATE;
  b->cancel = 1;
  return AR_PENDING;
}
int ar_borrow_ack(ArBorrow *b) {
  if (!b || !b->active || !b->cancel)
    return AR_STATE;
  if (AR_MUTANT != 9)
    b->active = 0;
  b->data = 0;
  return AR_CANCELLED;
}
