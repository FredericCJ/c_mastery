/* Hosted C17 library-contract adapter, not an original printf implementation.
 */
#include "format.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static struct lf_result outcome(size_t required, size_t cap)
{
    struct lf_result r = {required,
                          cap ? (required < cap ? required : cap - 1) : 0,
                          cap != 0, cap != 0 && required < cap};
    return r;
}

enum lf_status lf_prefix(lf_formatter f, void *ctx, char *d, size_t cap,
                         struct lf_result *out)
{
    if (!f || !out || (!d && cap))
        return LF_ARGUMENT;
    int n = f(ctx, d, cap);
#ifndef LF_MUTANT_SIGN
    if (n < 0)
        return LF_ENCODING;
#endif
    struct lf_result r = outcome((size_t)n, cap);
#ifdef LF_MUTANT_FULL
    r.complete = (size_t)n <= cap;
#endif
    *out = r;
    return r.complete ? LF_OK : LF_TRUNCATED;
}

enum lf_status lf_transaction(lf_formatter f, void *ctx, char *d, size_t cap,
                              struct lf_result *out)
{
    if (!f || !out || (!d && cap))
        return LF_ARGUMENT;
    int n = f(ctx, NULL, 0);
    if (n < 0)
        return LF_ENCODING;
    if ((size_t)n >= cap || (size_t)n >= 64)
        return LF_CAPACITY;
    char stage[64];
    for (size_t i = 0; i < 64; ++i)
        stage[i] = (char)0x5a;
#ifdef LF_MUTANT_COMMIT
    int actual = f(ctx, d, (size_t)n + 1);
#else
    int actual = f(ctx, stage, (size_t)n + 1);
#endif
    if (actual < 0)
        return LF_ENCODING;
    if (actual != n)
        return LF_BACKEND;
#ifndef LF_MUTANT_COMMIT
    if (stage[n] != 0)
        return LF_BACKEND;
    for (size_t i = 0; i <= (size_t)n; ++i)
        d[i] = stage[i];
#endif
    *out = outcome((size_t)n, cap);
    return LF_OK;
}

enum lf_status lf_parse_policy(const char *s, enum lf_policy *out)
{
    if (!s || !out)
        return LF_ARGUMENT;
    if (!strcmp(s, "text"))
    {
        *out = LF_TEXT;
        return LF_OK;
    }
    if (!strcmp(s, "u32"))
    {
        *out = LF_U32;
        return LF_OK;
    }
    return LF_ARGUMENT;
}

static int format_value(void *ctx, char *d, size_t n)
{
    const struct lf_value *v = ctx;
    return v->tag == LF_TEXT ? snprintf(d, n, "%s", v->data.text)
                             : snprintf(d, n, "%" PRIu32, v->data.u32);
}

enum lf_status lf_typed(enum lf_policy policy, struct lf_value v, char *d,
                        size_t n, struct lf_result *out)
{
    if ((policy != LF_TEXT && policy != LF_U32) || v.tag != policy ||
        (policy == LF_TEXT && !v.data.text))
        return LF_ARGUMENT;
    return lf_prefix(format_value, &v, d, n, out);
}
