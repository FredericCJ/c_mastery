#ifndef CMASTERY_ARCH_BORROW_H
#define CMASTERY_ARCH_BORROW_H
#include "arch.h"

struct ar_borrow
{
    const unsigned char *data;
    size_t length, offset;
    unsigned active, cancel;
};

int ar_borrow(struct ar_borrow *, const unsigned char *, size_t);
int ar_borrow_chunk(struct ar_borrow *, size_t, unsigned char *, size_t *);
int ar_borrow_cancel(struct ar_borrow *);
int ar_borrow_ack(struct ar_borrow *);
#endif
