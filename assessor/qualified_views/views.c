/*
 * Reference implementation of the qualified-views lab (labs/qualified_views).
 *
 * FV_MUTANT selects one deliberate defect for the mutation campaign; 0 is the
 * correct behaviour. Each hook is marked "mutant N" where it is applied.
 * The component calls no library function by name: the compiler may still
 * emit memcpy/memmove/memset, which the campaign audits.
 */
#include "views.h"

#ifndef FV_MUTANT
#define FV_MUTANT 0
#endif

enum
{
    RECORD_VALUES = 4 /* ints in struct fv_record.values */
};

struct fv_owner
{
    struct fv_record record;
};

static struct fv_owner sole_owner;

struct fv_view fv_initialize(struct fv_record *caller_owner)
{
    if (!caller_owner)
    {
        return (struct fv_view){NULL, 0};
    }

    /*
     * The literal is only a source for the copy; the view must point at
     * caller storage, never at the literal.
     */
    *caller_owner =
        (struct fv_record){.values = {[1] = 7, [3] = 9}, .link = NULL};
    return fv_view(caller_owner);
}

enum fv_status fv_reset(struct fv_record *owner)
{
    if (!owner)
    {
        return FV_INVALID;
    }

    /* {0} sets link to a null pointer value, not merely to zero bytes. */
    *owner = (struct fv_record){0};
    if (FV_MUTANT == 1)
    {
        /* mutant 1: reset leaves a stale value behind. */
        owner->values[3] = 1;
    }
    return FV_OK;
}

enum fv_status fv_update(struct fv_record *owner, size_t index, int value)
{
    if (!owner)
    {
        return FV_INVALID;
    }
    if (index >= RECORD_VALUES)
    {
        return FV_RANGE;
    }
    owner->values[index] = value;
    return FV_OK;
}

struct fv_view fv_view(const struct fv_record *owner)
{
    if (!owner)
    {
        return (struct fv_view){NULL, 0};
    }
    return (struct fv_view){owner->values, RECORD_VALUES};
}

/*
 * Copies count ints. The restrict qualifiers are the caller's promise that the
 * two ranges do not overlap; overlapping calls are undefined.
 */
static void copy_disjoint(int *restrict out, const int *restrict in,
                          size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        out[i] = in[i];
    }
}

enum fv_status fv_copy(struct fv_destination destination, struct fv_view source)
{
    int source_missing = source.length != 0 && !source.data;
    int destination_missing = destination.capacity != 0 && !destination.data;
    if (source_missing || destination_missing)
    {
        return FV_INVALID;
    }
    if (destination.capacity < source.length)
    {
        if (FV_MUTANT == 2 && destination.capacity)
        {
            /* mutant 2: a rejected copy still clobbers the destination. */
            destination.data[0] = 0;
        }
        return FV_FULL;
    }

    copy_disjoint(destination.data, source.data, source.length);
    return FV_OK;
}

/*
 * True when copying upward would overwrite source elements before they are
 * read: the destination starts inside the source slice.
 */
static int must_copy_downward(size_t destination, size_t source, size_t count)
{
    return destination > source && destination - source < count;
}

enum fv_status fv_move(int *base, size_t extent, size_t destination,
                       size_t source, size_t count)
{
    if (extent && !base)
    {
        return FV_INVALID;
    }

    /*
     * Check by subtraction after the position test: destination + count could
     * wrap, extent - destination cannot.
     */
    if (destination > extent || source > extent ||
        count > extent - destination || count > extent - source)
    {
        return FV_RANGE;
    }

    /* mutant 3: always copy upward, which corrupts overlapping moves. */
    if (FV_MUTANT != 3 && must_copy_downward(destination, source, count))
    {
        for (size_t i = count; i > 0; --i)
        {
            base[destination + i - 1] = base[source + i - 1];
        }
    }
    else
    {
        for (size_t i = 0; i < count; ++i)
        {
            base[destination + i] = base[source + i];
        }
    }
    return FV_OK;
}

struct fv_owner *fv_owner(void)
{
    return &sole_owner;
}

struct fv_view fv_owner_view(const struct fv_owner *owner)
{
    if (!owner)
    {
        return (struct fv_view){NULL, 0};
    }
    return fv_view(&owner->record);
}

int *fv_owner_mutable(struct fv_owner *owner)
{
    if (!owner)
    {
        return NULL;
    }
    return owner->record.values;
}

/*
 * Contract: status points to a live volatile unsigned. Two separate reads must
 * both reach memory, which is what the object-code inspection verifies.
 */
unsigned fv_device_twice(volatile unsigned *status)
{
    unsigned first = *status;
    /* mutant 4: reuse the first read, so the optimizer sees one load. */
    unsigned second = FV_MUTANT == 4 ? first : *status;
    return first ^ second;
}
