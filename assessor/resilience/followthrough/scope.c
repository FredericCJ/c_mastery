#include "scope.h"

int rzf_scope_retain(struct rzf_scope *scope, struct rzf_scope **out)
{
    if (!scope || !out)
    {
        return RZ_INVALID;
    }
    size_t n = atomic_load_explicit(&scope->references, memory_order_relaxed);
    do
    {
        if (n == SIZE_MAX)
        {
            return RZ_FULL;
        }
        if (n == 0)
        {
            return RZ_STALE;
        }
    } while (!atomic_compare_exchange_weak_explicit(&scope->references, &n,
                                                    n + 1, memory_order_relaxed,
                                                    memory_order_relaxed));
    *out = scope;
    return RZ_OK;
}

void rzf_scope_release(struct rzf_scope **owner)
{
    if (!owner || !*owner)
    {
        return;
    }
    struct rzf_scope *scope = *owner;
    *owner = NULL;
    while (scope)
    {
        if (atomic_fetch_sub_explicit(&scope->references, 1,
                                      memory_order_acq_rel) != 1)
        {
            return;
        }
        struct rzf_scope *parent = scope->parent;
        struct rzf_allocator a = scope->allocator;
        a.deallocate(a.context, scope);
        scope = parent;
    }
}

int rzf_scope_create(struct rzf_scope *parent, uint64_t now, uint64_t budget,
                     struct rzf_allocator a, struct rzf_scope **out)
{
    if (!out || !a.allocate || !a.deallocate)
    {
        return RZ_INVALID;
    }
    struct rz_scope checked;
    int status =
        rz_scope_init(&checked, parent ? &parent->core : NULL, now, budget);
    if (status != RZ_OK)
    {
        return status;
    }
    struct rzf_scope *owned_parent = NULL;
    if (parent)
    {
        status = rzf_scope_retain(parent, &owned_parent);
        if (status != RZ_OK)
        {
            return status;
        }
    }
    struct rzf_scope *scope = a.allocate(a.context, sizeof *scope);
    if (!scope)
    {
        rzf_scope_release(&owned_parent);
        return RZ_FULL;
    }
    (void)rz_scope_init(&scope->core, parent ? &parent->core : NULL, now,
                        budget);
    atomic_init(&scope->references, 1);
    scope->parent = owned_parent;
    scope->allocator = a;
    *out = scope;
    return RZ_OK;
}

int rzf_borrow_begin(struct rzf_borrow *borrow, struct rzf_scope *scope)
{
    if (!borrow || borrow->scope)
    {
        return RZ_INVALID;
    }
#if RZF_MUTANT == 12
    if (!scope)
    {
        return RZ_INVALID;
    }
    borrow->scope = scope;
    return RZ_OK;
#else
    return rzf_scope_retain(scope, &borrow->scope);
#endif
}

int rzf_borrow_ack(struct rzf_borrow *borrow)
{
    if (!borrow)
    {
        return RZ_INVALID;
    }
    if (!borrow->scope)
    {
        return RZ_STALE;
    }
    rzf_scope_release(&borrow->scope);
    return RZ_OK;
}

static int budget_valid(const struct rzf_budget *budget)
{
    return budget && budget->remaining &&
           budget->transit_bound < budget->remaining;
}

int rzf_budget_export(const struct rzf_scope *scope, uint64_t now,
                      uint64_t transit, struct rzf_budget *out)
{
    if (!scope || !out)
    {
        return RZ_INVALID;
    }
    uint64_t remaining;
    int status = rz_remaining(&scope->core, now, &remaining);
    if (status != RZ_OK)
    {
        return status;
    }
    if (transit >= remaining)
    {
        return RZ_EXPIRED;
    }
    *out = (struct rzf_budget){remaining, transit};
    return RZ_OK;
}

int rzf_budget_import(const struct rzf_budget *budget, struct rzf_scope *parent,
                      uint64_t now, uint64_t processing, struct rzf_allocator a,
                      struct rzf_scope **out)
{
    if (!out || !budget_valid(budget))
    {
        return RZ_INVALID;
    }
    uint64_t remaining = budget->remaining - budget->transit_bound;
    if (processing >= remaining)
    {
        return RZ_EXPIRED;
    }
#if RZF_MUTANT == 11
    remaining = budget->remaining;
#else
    remaining -= processing;
#endif
    return rzf_scope_create(parent, now, remaining, a, out);
}

int rzf_budget_encode(const struct rzf_budget *budget, unsigned char out[16])
{
    if (!out || !budget_valid(budget))
    {
        return RZ_INVALID;
    }
    uint64_t values[2] = {budget->remaining, budget->transit_bound};
    for (unsigned word = 0; word < 2; ++word)
    {
        for (unsigned byte = 0; byte < 8; ++byte)
        {
            out[8 * word + byte] =
                (unsigned char)(values[word] >> (56 - 8 * byte));
        }
    }
    return RZ_OK;
}

int rzf_budget_decode(const unsigned char wire[16], struct rzf_budget *out)
{
    if (!wire || !out)
    {
        return RZ_INVALID;
    }
    uint64_t values[2] = {0, 0};
    for (unsigned word = 0; word < 2; ++word)
    {
        for (unsigned byte = 0; byte < 8; ++byte)
        {
            values[word] = (values[word] << 8) | wire[8 * word + byte];
        }
    }
    struct rzf_budget value = {values[0], values[1]};
    if (!budget_valid(&value))
    {
        return RZ_INVALID;
    }
    *out = value;
    return RZ_OK;
}
