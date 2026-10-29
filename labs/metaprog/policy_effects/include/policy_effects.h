#ifndef CMASTERY_META_POLICY_EFFECTS_H
#define CMASTERY_META_POLICY_EFFECTS_H
#include <stddef.h>
#include <stdint.h>

enum
{
    MPP_OK,
    MPP_INVALID,
    MPP_ACQUIRE,
    MPP_PROJECT,
    MPP_RANGE
};

struct mpp_item
{
    void *owner;
    uint32_t raw;
};

struct mpp_ops
{
    void *context;
    int (*acquire)(void *, size_t, struct mpp_item *);
    int (*project)(void *, const struct mpp_item *, uint32_t *);
    void (*release)(void *, struct mpp_item *);
};

/* 0..32 elements, live disjoint output, valid callbacks even for empty input.
   Acquire returns0 on success; failure may leave a valid partial owner. Release
   accepts empty/partial/full item, cannot fail and leaves empty. Projection
   returns0 on success; its output is ignored on failure. All effects ordered
   acquire(i),project(i),release(i),arithmetic(i); acquisition failure skips only
   project, then releases. Callbacks return normally, do not reenter/mutate core
   temporaries/output or retain borrowed item/output pointers. */
int mpp_dynamic(size_t, uint32_t, unsigned, struct mpp_ops, uint32_t *);
int mpp_reject(size_t, uint32_t, struct mpp_ops, uint32_t *);
int mpp_saturate(size_t, uint32_t, struct mpp_ops, uint32_t *);
#endif
