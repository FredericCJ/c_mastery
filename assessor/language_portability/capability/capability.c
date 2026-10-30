#include "capability.h"
#include <limits.h>
#include <inttypes.h>
#include <stdio.h>

/* Build an optional value. An absent value is stored as 0 whatever the caller
   passed, so equal absent records compare equal bytewise. Returns 1, or 0
   (null out, present not 0/1) leaving *out unchanged. */
int lp_record(unsigned present, intmax_t value, struct lp_value *out)
{
    if (!out || present > 1)
    {
        return 0;
    }
    *out = (struct lp_value){.present = present,
                             .value = (present || LP_MUTANT == 4) ? value : 0};
    return 1;
}

enum
{
    STAGING_BYTES = 128,
    TRANSFORM_LIMIT = 16,
    CAPABILITY_MASK = 15
};

/* Render value as decimal text or "empty" into out (capacity_bytes includes
   the terminator). Text is staged first so out is unchanged on any refusal.
   Returns 1, or 0 for a null out, a malformed value or a too-small buffer. */
int lp_format(struct lp_value value, char *out, size_t capacity_bytes)
{
    char staged[STAGING_BYTES];
    if (!out || value.present > 1)
    {
        return 0;
    }
    int n;
    if (value.present)
    {
        n = snprintf(staged, sizeof staged, "%" PRIdMAX, value.value);
    }
    else
    {
        n = snprintf(staged, sizeof staged, "empty");
    }
    if (n < 0 || (size_t)n >= sizeof staged || (size_t)n >= capacity_bytes)
    {
        return 0;
    }
    for (size_t i = 0; i <= (size_t)n; i++)
    {
        out[i] = staged[i];
    }
    return 1;
}

/* destination[i] = 2 * source[i] for n <= 16 elements. Every element is
   range-checked before the first store, so a refusal leaves destination
   untouched. The spans must not overlap (restrict). Returns 1, or 0. */
int lp_transform(const int *restrict source, int *restrict destination,
                 size_t n)
{
    if (n > TRANSFORM_LIMIT || (n && (!source || !destination)))
    {
        return 0;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (source[i] > INT_MAX / 2 || source[i] < INT_MIN / 2)
        {
            return 0;
        }
#if LP_MUTANT == 1
        destination[i] = source[i] * 2;
#endif
    }
    for (size_t i = 0; i < n; i++)
    {
        destination[i] = source[i] * 2;
    }
    return 1;
}

/* *usable is 1 when every required capability bit is observed. Bits:
   language 1, header 2, link 4, run 8. Returns 0 for a null usable or a mask
   above 15, leaving *usable unchanged. */
int lp_capability(unsigned required, unsigned observed, unsigned *usable)
{
    if (!usable || required > CAPABILITY_MASK || observed > CAPABILITY_MASK)
    {
        return 0;
    }
#if LP_MUTANT == 2
    *usable = (observed & 1u) != 0;
#else
    *usable = (required & observed) == required;
#endif
    return 1;
}
