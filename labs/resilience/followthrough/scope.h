#ifndef CMASTERY_RESILIENCE_OWNED_SCOPE_H
#define CMASTERY_RESILIENCE_OWNED_SCOPE_H
#include "resilience.h"

typedef struct {
    void *context;
    void *(*allocate)(void *, size_t);
    void (*deallocate)(void *, void *);
} RzfAllocator;

/* Immutable tree edges own one reference. A raw pointer is not an owner. */
typedef struct RzfScope {
    RZScope core;
    atomic_size_t references;
    struct RzfScope *parent;
    RzfAllocator allocator;
} RzfScope;

typedef struct { RzfScope *scope; } RzfBorrow;
typedef struct { uint64_t remaining, transit_bound; } RzfBudget;

int rzf_scope_create(RzfScope *, uint64_t, uint64_t, RzfAllocator, RzfScope **);
int rzf_scope_retain(RzfScope *, RzfScope **);
void rzf_scope_release(RzfScope **);
int rzf_borrow_begin(RzfBorrow *, RzfScope *);
int rzf_borrow_ack(RzfBorrow *);
int rzf_budget_export(const RzfScope *, uint64_t, uint64_t, RzfBudget *);
int rzf_budget_import(const RzfBudget *, RzfScope *, uint64_t, uint64_t,
                      RzfAllocator, RzfScope **);
int rzf_budget_encode(const RzfBudget *, unsigned char [16]);
int rzf_budget_decode(const unsigned char [16], RzfBudget *);
#endif
