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

int lc_utf_feed(struct lc_utf *s, const unsigned char *p, size_t n)
{
    if (!s || (n && !p))
        return LC_INVALID;
    if (n > 64 - s->bytes)
        return LC_RANGE;
    struct lc_utf next = *s;
    if (LC_MUTANT == 2)
        next.remaining = 0;
    for (size_t i = 0; i < n; ++i)
    {
        unsigned c = p[i];
        if (!next.remaining)
        {
            if (c < 128)
            {
                ++next.scalars;
                next.scalar = 0;
                next.minimum = 0;
            }
            else if (c >= 0xc2 && c <= 0xdf)
            {
                next.scalar = c & 31;
                next.minimum = 0x80;
                next.remaining = 1;
            }
            else if (c >= 0xe0 && c <= 0xef)
            {
                next.scalar = c & 15;
                next.minimum = 0x800;
                next.remaining = 2;
            }
            else if (c >= 0xf0 && c <= 0xf4)
            {
                next.scalar = c & 7;
                next.minimum = 0x10000;
                next.remaining = 3;
            }
            else
                return LC_ENCODING;
        }
        else
        {
            if (c < 128 || c > 191)
                return LC_ENCODING;
            next.scalar = next.scalar * 64 + (c & 63);
            --next.remaining;
            if (!next.remaining)
            {
                if (LC_MUTANT != 1 &&
                    (next.scalar < next.minimum || next.scalar > 0x10ffff ||
                     (next.scalar >= 0xd800 && next.scalar <= 0xdfff)))
                    return LC_ENCODING;
                ++next.scalars;
                next.scalar = next.minimum = 0;
            }
        }
        if (next.remaining && LC_MUTANT != 1)
        {
            unsigned shift = 6 * next.remaining;
            uint32_t low = next.scalar << shift,
                     high = ((next.scalar + 1) << shift) - 1;
            if (high < next.minimum || low > 0x10ffff ||
                (low >= 0xd800 && high <= 0xdfff))
                return LC_ENCODING;
        }
        ++next.bytes;
    }
    if (LC_MUTANT == 4)
        next.scalars = next.bytes;
    *s = next;
    return LC_OK;
}

int lc_utf_finish(const struct lc_utf *s, size_t *out)
{
    if (!s || !out)
        return LC_INVALID;
    if (s->remaining)
        return LC_INCOMPLETE;
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
        return LC_INVALID;
    if (s->poisoned)
        return LC_POISONED;
    if (LC_MUTANT == 3)
        s->state = (mbstate_t){0};
    wchar_t value = 0;
    size_t result = mbrtowc(&value, p, n, &s->state);
    if (result == (size_t)-1)
    {
        s->poisoned = 1;
        return LC_ENCODING;
    }
    if (result == (size_t)-2)
        return LC_INCOMPLETE;
    *out = value;
    *used = result;
    return LC_OK;
}
