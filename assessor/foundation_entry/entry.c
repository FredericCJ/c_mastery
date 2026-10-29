#include "entry.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8 && INT_MAX == INT32_MAX, "explicit entry profile");
#ifndef FE_MUTANT
#define FE_MUTANT 0
#endif
static int add(int32_t a, int32_t b, int32_t *out)
{
    if ((b > 0 && a > INT32_MAX - b) || (b < 0 && a < INT32_MIN - b))
        return 0;
    *out = a + b;
    return 1;
}

enum fe_status fe_fixed(const int32_t *in, size_t n, int32_t *out, size_t cap)
{
    enum f_status r = f_prefix(in, n, out, cap);
    return r == F_OK      ? FE_OK
           : r == F_FULL  ? FE_FULL
           : r == F_RANGE ? FE_RANGE
                          : FE_INVALID;
}

static void event(struct fe_trace *t, unsigned entering, size_t i,
                  int32_t value)
{
    t->event[t->count++] = (struct fe_event){entering, i, value};
}

static enum fe_status recursive(const int32_t *p, size_t n, size_t i,
                                int32_t *out, struct fe_trace *t)
{
    event(t, 1, i, 0);
    if (t->maximum_live < i + 1)
        t->maximum_live = i + 1;
    int32_t value = 0;
    if (i != n)
    {
        enum fe_status r = recursive(p, n, i + 1, &value, t);
        if (r != FE_OK)
            return r;
        if (!add(p[i], value, &value))
            return FE_RANGE;
    }
    event(t, 0, i, value);
    *out = value;
    return FE_OK;
}

static enum fe_status trace_check(const int32_t *p, size_t n, int32_t *out,
                                  struct fe_trace *t)
{
    if (!out || !t || (n && !p))
        return FE_INVALID;
    if (n > 16)
        return FE_FULL;
    t->count = t->maximum_live = 0;
    return FE_OK;
}

enum fe_status fe_recursive(const int32_t *p, size_t n, int32_t *out,
                            struct fe_trace *t)
{
    enum fe_status r = trace_check(p, n, out, t);
    if (r != FE_OK)
        return r;
    int32_t value;
    r = recursive(p, n, 0, &value, t);
    if (r == FE_OK)
        *out = value;
    return r;
}

enum fe_status fe_explicit_stack(const int32_t *p, size_t n, int32_t *out,
                                 struct fe_trace *t)
{
    enum fe_status r = trace_check(p, n, out, t);
    if (r != FE_OK)
        return r;

    /* One record for each suspended recursive activation, including base case. */
    struct ad_frame
    {
        size_t index;
    } frames[17];

    size_t live = 0;
    for (size_t i = 0; i <= n; ++i)
    {
        frames[live++].index = i;
        event(t, 1, i, 0);
    }
    t->maximum_live = live;
    int32_t value = 0;
    while (live)
    {
        size_t i = frames[--live].index;
        if (i != n && !add(p[i], value, &value))
            return FE_RANGE;
        if (FE_MUTANT != 1 || i != n)
            event(t, 0, i, value);
    }
    *out = value;
    return FE_OK;
}

static enum fe_status extent(const int32_t *p, size_t n, int32_t *out,
                             size_t cap)
{
    if (n > cap)
        return FE_FULL;
    if (n && (!p || !out))
        return FE_INVALID;
    return FE_OK;
}

enum fe_status fe_two_pass(const int32_t *p, size_t n, int32_t *out, size_t cap)
{
    enum fe_status r = extent(p, n, out, cap);
    if (r != FE_OK)
        return r;
    int32_t total = 0;
    for (size_t i = 0; i < n; ++i)
    {
        if (!add(total, p[i], &total))
            return FE_RANGE;
        if (FE_MUTANT == 2)
            out[i] = total;
    }
    total = 0;
    for (size_t i = 0; i < n; ++i)
    {
        /* First pass proved each addition, inputs unchanged or exact in-place. */
        total += p[i];
        out[i] = total;
    }
    return FE_OK;
}

enum fe_status fe_staged(const int32_t *p, size_t n, int32_t *out, size_t cap,
                         struct fe_allocator a)
{
    enum fe_status r = extent(p, n, out, cap);
    if (r != FE_OK || !n)
        return r;
    if (n > SIZE_MAX / sizeof(int32_t))
        return FE_RANGE;
    if (!a.take || !a.release)
        return FE_INVALID;
    int32_t *stage = a.take(a.context, n);
    if (!stage)
        return FE_ALLOC;
    int32_t total = 0;
    for (size_t i = 0; i < n; ++i)
    {
        if (!add(total, p[i], &total))
        {
            if (FE_MUTANT != 3)
                a.release(a.context, stage);
            return FE_RANGE;
        }
        stage[i] = total;
    }
    for (size_t i = 0; i < n; ++i)
        out[i] = stage[i];
    a.release(a.context, stage);
    return FE_OK;
}

enum fe_status fe_maximum(const int32_t *p, size_t n, int32_t *out)
{
    if (!out || (n && !p))
        return FE_INVALID;
    if (!n)
    {
        if (FE_MUTANT == 4)
            *out = 0;
        return FE_EMPTY;
    }
    int32_t max = p[0];
    for (size_t i = 1; i < n; ++i)
        if (p[i] > max)
            max = p[i];
    *out = max;
    return FE_OK;
}

enum fe_status fe_encode24(uint32_t v, unsigned char *out, size_t cap)
{
    if (!out)
        return FE_INVALID;
    if (cap < 3)
        return FE_FULL;
    if (v > UINT32_C(0xffffff))
        return FE_RANGE;
    for (unsigned i = 0; i < 3; ++i)
        out[i] = (unsigned char)(v >> ((FE_MUTANT == 5 ? i : 2 - i) * 8));
    return FE_OK;
}

int fe_any(uint8_t bits, uint8_t mask)
{
    return (bits & mask) != 0;
}

int fe_all(uint8_t bits, uint8_t mask)
{
    return (bits & mask) == mask;
}

int fe_forbidden_clear(uint8_t bits, uint8_t forbidden)
{
    return FE_MUTANT == 6 ? fe_all(bits, forbidden) : (bits & forbidden) == 0;
}

int32_t fe_saturating_add(int32_t a, int32_t b)
{
    int32_t out;
    return add(a, b, &out) ? out : b > 0 ? INT32_MAX : INT32_MIN;
}
