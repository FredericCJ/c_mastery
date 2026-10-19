#ifndef CMASTERY_ARCH_BORROW_H
#define CMASTERY_ARCH_BORROW_H
#include "arch.h"
typedef struct {
  const unsigned char *data;
  size_t length, offset;
  unsigned active, cancel;
} ArBorrow;
int ar_borrow(ArBorrow *, const unsigned char *, size_t);
int ar_borrow_chunk(ArBorrow *, size_t, unsigned char *, size_t *);
int ar_borrow_cancel(ArBorrow *);
int ar_borrow_ack(ArBorrow *);
#endif
