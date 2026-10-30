#include "context.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "octet profile");
#ifndef LC_MUTANT
#define LC_MUTANT 0
#endif
void lc_utf_init(struct lc_utf *s)
{
    *s = (struct lc_utf){0, 0, 0, 0, 0};
}

/* Unicode scalar-value limits (code points, not bytes). */
enum
{
    UTF_MAX_SCALAR = 0x10ffff,
    UTF_SURROGATE_FIRST = 0xd800,
    UTF_SURROGATE_LAST = 0xdfff,
    UTF_BYTE_LIMIT = 64
};

static int is_surrogate(uint32_t scalar)
{
    return scalar >= UTF_SURROGATE_FIRST && scalar <= UTF_SURROGATE_LAST;
}

/* Accept a lead byte: complete a one-byte scalar or open a multi-byte one.
   Stray continuation bytes (0x80-0xbf), overlong leads (0xc0, 0xc1) and
   leads above 0xf4 are encoding errors.
   Returns LC_OK or LC_ENCODING; modifies only the working copy. */
static int utf_lead(struct lc_utf *next, unsigned c)
{
    if (c < 0x80)
    {
        ++next->scalars;
        next->scalar = 0;
        next->minimum = 0;
    }
    else if (c >= 0xc2 && c <= 0xdf)
    {
        next->scalar = c & 0x1f;
        next->minimum = 0x80;
        next->remaining = 1;
    }
    else if (c >= 0xe0 && c <= 0xef)
    {
        next->scalar = c & 0x0f;
        next->minimum = 0x800;
        next->remaining = 2;
    }
    else if (c >= 0xf0 && c <= 0xf4)
    {
        next->scalar = c & 0x07;
        next->minimum = 0x10000;
        next->remaining = 3;
    }
    else
    {
        return LC_ENCODING;
    }
    return LC_OK;
}

/* Accept a continuation byte (10xxxxxx). When the sequence completes, reject
   overlong forms, values past U+10FFFF and surrogates. */
static int utf_continuation(struct lc_utf *next, unsigned c)
{
    if (c < 0x80 || c > 0xbf)
    {
        return LC_ENCODING;
    }
    next->scalar = next->scalar * 64 + (c & 0x3f);
    --next->remaining;
    if (next->remaining)
    {
        return LC_OK;
    }
    if (LC_MUTANT != 1 &&
        (next->scalar < next->minimum || next->scalar > UTF_MAX_SCALAR ||
         is_surrogate(next->scalar)))
    {
        return LC_ENCODING;
    }
    ++next->scalars;
    next->scalar = 0;
    next->minimum = 0;
    return LC_OK;
}

/* Reject an unfinished sequence as soon as no completion can be valid: the
   prefix defines the interval [low, high] of scalars still reachable. This
   makes failure independent of how the caller splits the input. */
static int utf_prefix_viable(const struct lc_utf *next)
{
    unsigned shift_bits = 6 * next->remaining;
    uint32_t low = next->scalar << shift_bits;
    uint32_t high = ((next->scalar + 1) << shift_bits) - 1;
    if (high < next->minimum || low > UTF_MAX_SCALAR)
    {
        return 0;
    }
    return !(low >= UTF_SURROGATE_FIRST && high <= UTF_SURROGATE_LAST);
}

/* Feed n bytes (at most UTF_BYTE_LIMIT in total). Atomic: on any error the
   decoder state is unchanged. Returns LC_OK, LC_INVALID (null arguments),
   LC_RANGE (byte budget exceeded) or LC_ENCODING. */
int lc_utf_feed(struct lc_utf *s, const unsigned char *p, size_t n)
{
    if (!s || (n && !p))
    {
        return LC_INVALID;
    }
    if (n > UTF_BYTE_LIMIT - s->bytes)
    {
        return LC_RANGE;
    }
    struct lc_utf next = *s;
    if (LC_MUTANT == 2)
    {
        next.remaining = 0;
    }
    for (size_t i = 0; i < n; ++i)
    {
        int status = next.remaining ? utf_continuation(&next, p[i])
                                    : utf_lead(&next, p[i]);
        if (status != LC_OK)
        {
            return status;
        }
        if (next.remaining && LC_MUTANT != 1 && !utf_prefix_viable(&next))
        {
            return LC_ENCODING;
        }
        ++next.bytes;
    }
    if (LC_MUTANT == 4)
    {
        next.scalars = next.bytes;
    }
    *s = next;
    return LC_OK;
}

int lc_utf_finish(const struct lc_utf *s, size_t *out)
{
    if (!s || !out)
    {
        return LC_INVALID;
    }
    if (s->remaining)
    {
        return LC_INCOMPLETE;
    }
    *out = s->scalars;
    return LC_OK;
}

void lc_locale_init(struct lc_locale *s)
{
    *s = (struct lc_locale){0};
}

int lc_locale_one(struct lc_locale *s, const char *p, size_t n, wchar_t *out,
                  size_t *used)
{
    if (!s || !p || !out || !used)
    {
        return LC_INVALID;
    }
    if (s->poisoned)
    {
        return LC_POISONED;
    }
    if (LC_MUTANT == 3)
    {
        s->state = (mbstate_t){0};
    }
    wchar_t value = 0;
    size_t result = mbrtowc(&value, p, n, &s->state);
    if (result == (size_t)-1)
    {
        s->poisoned = 1;
        return LC_ENCODING;
    }
    if (result == (size_t)-2)
    {
        return LC_INCOMPLETE;
    }
    *out = value;
    *used = result;
    return LC_OK;
}
