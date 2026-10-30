/*
 * Reference implementation of the clinic laboratory (private assessor copy).
 * Contracts are stated in labs/object_followthrough/clinic/clinic.h and
 * CONTRACTS.md; this file adds implementation notes only.
 *
 * Test hooks: each MC_MUTANT_* macro breaks exactly one contract so the
 * campaign can prove that the tests reject the breakage.
 */
#include "clinic.h"

int mc_acquire(struct mc_owner *owner, struct mc_allocator allocator, int value)
{
    if (owner == NULL || owner->value != NULL || allocator.allocate == NULL ||
        allocator.release == NULL)
    {
        return -1;
    }
    int *block = allocator.allocate(allocator.context, sizeof *block);
    if (block == NULL)
    {
        return -1;
    }
    /* Publish into the owner only after the block is initialized. */
    *block = value;
    owner->value = block;
    owner->allocation = allocator;
    return 0;
}

int mc_read(const struct mc_owner *owner, int *out)
{
    if (owner == NULL || owner->value == NULL || out == NULL)
    {
        return -1;
    }
    *out = *owner->value;
    return 0;
}

int mc_snapshot(const struct mc_owner *owner, int *out)
{
#ifdef MC_MUTANT_ESCAPE
    if (owner == NULL || owner->value == NULL || out == NULL)
    {
        return -1;
    }
    *out = 0;
    return 0;
#else
    /* A snapshot is a copy of the value, so it is mc_read() by definition. */
    return mc_read(owner, out);
#endif
}

/*
 * Order matters: the owner gives up the pointer before the callback runs, so
 * the owner never holds a pointer the allocator may already have freed.
 */
int mc_release(struct mc_owner *owner)
{
    if (owner == NULL || owner->value == NULL)
    {
#ifdef MC_MUTANT_RELEASE
        return 0;
#else
        return -1;
#endif
    }
    int *block = owner->value;
    owner->value = NULL;
    ++owner->releases;
    owner->allocation.release(owner->allocation.context, block);
    return 0;
}

enum fd_status mc_move(unsigned char *base, size_t length_bytes, size_t dst,
                       size_t src, size_t count)
{
    return fd_move(base, length_bytes, dst, src, count);
}

int mc_member(const struct mc_container *owner, const int *member, int *out)
{
    if (owner == NULL || member == NULL || out == NULL)
    {
        return -1;
    }
    /* The expected address is derived from the whole object, never by
     * subtracting an offset from the candidate. */
    const unsigned char *whole = (const unsigned char *)owner;
    const size_t member_offset = offsetof(struct mc_container, member);
    const int *expected = (const int *)(const void *)(whole + member_offset);
#ifndef MC_MUTANT_MEMBER
    if (member != expected)
    {
        return -1;
    }
#else
    (void)expected;
#endif
    *out = owner->member;
    return 0;
}

int mc_initial(const struct mc_first *owner, const int *member, int *out)
{
    if (owner == NULL || member == NULL || out == NULL ||
        member != &owner->first)
    {
        return -1;
    }
    /* The first member of a structure may be converted back to the
     * structure pointer; the candidate was just proven to be that member. */
    const struct mc_first *recovered =
        (const struct mc_first *)(const void *)member;
    if (recovered != owner)
    {
        return -1;
    }
    *out = recovered->first;
    return 0;
}
