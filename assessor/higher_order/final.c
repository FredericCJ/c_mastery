#include "higher_order.h"
#include <limits.h>
#ifndef HO_MUTANT
#define HO_MUTANT 0
#endif
static int e_lit(int32_t n, he_int *out)
{
    if (!out)
    {
        return HO_INVALID;
    }
    *out = n;
    return HO_OK;
}

static int e_add(he_int a, he_int b, he_int *out)
{
    if (!out)
    {
        return HO_INVALID;
    }
    int64_t n = (int64_t)a + (HO_MUTANT == 3 ? -(int64_t)b : b);
    if (n < INT32_MIN || n > INT32_MAX)
    {
        return HO_RANGE;
    }
    *out = (int32_t)n;
    return HO_OK;
}

static int e_lam(he_body_fn body, void *context, struct he_fun *out)
{
    if (!body || !out)
    {
        return HO_INVALID;
    }
    *out = (struct he_fun){body, context};
    return HO_OK;
}

static int e_app(struct he_fun f, he_int x, he_int *out)
{
    if (!f.body || !out)
    {
        return HO_INVALID;
    }
    he_int r;
    int s = f.body(f.context, x, &r);
    if (HO_MUTANT == 4 && s == HO_OK)
    {
        s = f.body(f.context, x, &r);
    }
    if (s == HO_OK)
    {
        *out = r;
    }
    return s;
}

struct he_ops ho_eval_ops(void)
{
    return (struct he_ops){e_lit, e_add, e_lam, e_app};
}

static int append(struct hp_int *p, const char *s, size_t n)
{
    if (n > 127 - p->length)
    {
        return HO_FULL;
    }
    for (size_t i = 0; i < n; ++i)
    {
        p->data[p->length + i] = s[i];
    }
    p->length += n;
    p->data[p->length] = 0;
    return HO_OK;
}

static int p_lit(int32_t n, struct hp_int *out)
{
    if (!out)
    {
        return HO_INVALID;
    }
    struct hp_int r = {{0}, 0};
    char digits[11];
    unsigned count = 0;
    int64_t value = n;
    if (value < 0)
    {
        r.data[r.length++] = '-';
        value = -value;
    }
    do
    {
        digits[count++] = (char)('0' + value % 10);
        value /= 10;
    } while (value);
    while (count)
    {
        r.data[r.length++] = digits[--count];
    }
    r.data[r.length] = 0;
    *out = r;
    return HO_OK;
}

static int p_add(struct hp_int a, struct hp_int b, struct hp_int *out)
{
    if (!out)
    {
        return HO_INVALID;
    }
    struct hp_int r = {{0}, 0};
    int s = append(&r, "(", 1);
    if (!s)
    {
        s = append(&r, a.data, a.length);
    }
    if (!s)
    {
        s = append(&r, "+", 1);
    }
    if (!s)
    {
        s = append(&r, b.data, b.length);
    }
    if (!s)
    {
        s = append(&r, ")", 1);
    }
    if (!s)
    {
        *out = r;
    }
    return s;
}

static int p_lam(hp_body_fn body, void *context, struct hp_fun *out)
{
    if (!body || !out)
    {
        return HO_INVALID;
    }
    struct hp_int variable = {{'x', 0}, 1}, result, r = {{0}, 0};
    int s = body(context, variable, &result);
    if (!s)
    {
        s = append(&r, "(lambda x.", 10);
    }
    if (!s)
    {
        s = append(&r, result.data, result.length);
    }
    if (!s)
    {
        s = append(&r, ")", 1);
    }
    if (!s)
    {
        struct hp_fun f = {{0}, r.length};
        for (size_t i = 0; i <= r.length; ++i)
        {
            f.data[i] = r.data[i];
        }
        *out = f;
    }
    return s;
}

static int p_app(struct hp_fun f, struct hp_int x, struct hp_int *out)
{
    if (!out)
    {
        return HO_INVALID;
    }
    struct hp_int r = {{0}, 0};
    int s = append(&r, "(", 1);
    if (!s)
    {
        s = append(&r, f.data, f.length);
    }
    if (!s)
    {
        s = append(&r, " ", 1);
    }
    if (!s)
    {
        s = append(&r, x.data, x.length);
    }
    if (!s)
    {
        s = append(&r, ")", 1);
    }
    if (!s)
    {
        *out = r;
    }
    return s;
}

struct hp_ops ho_print_ops(void)
{
    return (struct hp_ops){p_lit, p_add, p_lam, p_app};
}
