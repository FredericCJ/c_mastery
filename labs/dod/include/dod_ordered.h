#ifndef CMASTERY_DOD_ORDERED_H
#define CMASTERY_DOD_ORDERED_H
#include "dod.h"

struct d_ordered
{
    struct d_sparse pool;
    unsigned head, tail, next[4], previous[4], version, link_writes,
        iteration_steps;
};

struct d_ordered_iter
{
    const struct d_ordered *owner;
    unsigned version, next;
};

void d_order_init(struct d_ordered *);
int d_order_add(struct d_ordered *, int, struct d_handle *);
int d_order_remove(struct d_ordered *, struct d_handle);
int d_order_begin(const struct d_ordered *, struct d_ordered_iter *);
int d_order_next(struct d_ordered *, struct d_ordered_iter *, struct d_handle *,
                 const int **, unsigned *has_value);
#endif
