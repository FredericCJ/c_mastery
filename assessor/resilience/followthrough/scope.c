#include "scope.h"

int rzf_scope_retain(RzfScope *scope, RzfScope **out) {
    if (!scope || !out) return RZ_INVALID;
    size_t n = atomic_load_explicit(&scope->references, memory_order_relaxed);
    do {
        if (n == SIZE_MAX) return RZ_FULL;
        if (n == 0) return RZ_STALE;
    } while (!atomic_compare_exchange_weak_explicit(&scope->references, &n, n + 1,
                                                   memory_order_relaxed, memory_order_relaxed));
    *out = scope;
    return RZ_OK;
}

void rzf_scope_release(RzfScope **owner) {
    if (!owner || !*owner) return;
    RzfScope *scope = *owner;
    *owner = NULL;
    while (scope) {
        if (atomic_fetch_sub_explicit(&scope->references, 1, memory_order_acq_rel) != 1)
            return;
        RzfScope *parent = scope->parent;
        RzfAllocator a = scope->allocator;
        a.deallocate(a.context, scope);
        scope = parent;
    }
}

int rzf_scope_create(RzfScope *parent, uint64_t now, uint64_t budget,
                     RzfAllocator a, RzfScope **out) {
    if (!out || !a.allocate || !a.deallocate) return RZ_INVALID;
    RZScope checked;
    int status = rz_scope_init(&checked, parent ? &parent->core : NULL, now, budget);
    if (status != RZ_OK) return status;
    RzfScope *owned_parent = NULL;
    if (parent) {
        status = rzf_scope_retain(parent, &owned_parent);
        if (status != RZ_OK) return status;
    }
    RzfScope *scope = a.allocate(a.context, sizeof *scope);
    if (!scope) {
        rzf_scope_release(&owned_parent);
        return RZ_FULL;
    }
    (void)rz_scope_init(&scope->core, parent ? &parent->core : NULL, now, budget);
    atomic_init(&scope->references, 1);
    scope->parent = owned_parent;
    scope->allocator = a;
    *out = scope;
    return RZ_OK;
}

int rzf_borrow_begin(RzfBorrow *borrow, RzfScope *scope) {
    if (!borrow || borrow->scope) return RZ_INVALID;
#if RZF_MUTANT == 12
    if (!scope) return RZ_INVALID;
    borrow->scope = scope;
    return RZ_OK;
#else
    return rzf_scope_retain(scope, &borrow->scope);
#endif
}

int rzf_borrow_ack(RzfBorrow *borrow) {
    if (!borrow) return RZ_INVALID;
    if (!borrow->scope) return RZ_STALE;
    rzf_scope_release(&borrow->scope);
    return RZ_OK;
}

static int budget_valid(const RzfBudget *budget) {
    return budget && budget->remaining && budget->transit_bound < budget->remaining;
}

int rzf_budget_export(const RzfScope *scope, uint64_t now, uint64_t transit,
                      RzfBudget *out) {
    if (!scope || !out) return RZ_INVALID;
    uint64_t remaining;
    int status = rz_remaining(&scope->core, now, &remaining);
    if (status != RZ_OK) return status;
    if (transit >= remaining) return RZ_EXPIRED;
    *out = (RzfBudget){remaining, transit};
    return RZ_OK;
}

int rzf_budget_import(const RzfBudget *budget, RzfScope *parent, uint64_t now,
                      uint64_t processing, RzfAllocator a, RzfScope **out) {
    if (!out || !budget_valid(budget)) return RZ_INVALID;
    uint64_t remaining = budget->remaining - budget->transit_bound;
    if (processing >= remaining) return RZ_EXPIRED;
#if RZF_MUTANT == 11
    remaining = budget->remaining;
#else
    remaining -= processing;
#endif
    return rzf_scope_create(parent, now, remaining, a, out);
}

int rzf_budget_encode(const RzfBudget *budget, unsigned char out[16]) {
    if (!out || !budget_valid(budget)) return RZ_INVALID;
    uint64_t values[2] = {budget->remaining, budget->transit_bound};
    for (unsigned word = 0; word < 2; ++word)
        for (unsigned byte = 0; byte < 8; ++byte)
            out[8 * word + byte] = (unsigned char)(values[word] >> (56 - 8 * byte));
    return RZ_OK;
}

int rzf_budget_decode(const unsigned char wire[16], RzfBudget *out) {
    if (!wire || !out) return RZ_INVALID;
    uint64_t values[2] = {0, 0};
    for (unsigned word = 0; word < 2; ++word)
        for (unsigned byte = 0; byte < 8; ++byte)
            values[word] = (values[word] << 8) | wire[8 * word + byte];
    RzfBudget value = {values[0], values[1]};
    if (!budget_valid(&value)) return RZ_INVALID;
    *out = value;
    return RZ_OK;
}
