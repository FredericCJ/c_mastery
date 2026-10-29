#ifndef CMASTERY_RESILIENCE_OWNED_SCOPE_H
#define CMASTERY_RESILIENCE_OWNED_SCOPE_H
#include "resilience.h"

struct rzf_allocator
{
    void *context;
    void *(*allocate)(void *, size_t);
    void (*deallocate)(void *, void *);
};

/* Immutable tree edges own one reference. A raw pointer is not an owner. */
struct rzf_scope
{
    struct rz_scope core;
    atomic_size_t references;
    struct rzf_scope *parent;
    struct rzf_allocator allocator;
};

struct rzf_borrow
{
    struct rzf_scope *scope;
};

struct rzf_budget
{
    uint64_t remaining, transit_bound;
};

int rzf_scope_create(struct rzf_scope *, uint64_t, uint64_t,
                     struct rzf_allocator, struct rzf_scope **);
int rzf_scope_retain(struct rzf_scope *, struct rzf_scope **);
void rzf_scope_release(struct rzf_scope **);
int rzf_borrow_begin(struct rzf_borrow *, struct rzf_scope *);
int rzf_borrow_ack(struct rzf_borrow *);
int rzf_budget_export(const struct rzf_scope *, uint64_t, uint64_t,
                      struct rzf_budget *);
int rzf_budget_import(const struct rzf_budget *, struct rzf_scope *, uint64_t,
                      uint64_t, struct rzf_allocator, struct rzf_scope **);
int rzf_budget_encode(const struct rzf_budget *, unsigned char[16]);
int rzf_budget_decode(const unsigned char[16], struct rzf_budget *);
#endif
