#include "arch.h"
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
int ar_config(const struct ar_field *f, size_t n, struct ar_config *out)
{
    if (!f || !out || n > 16)
        return AR_INVALID;
    unsigned seen = 0;
    uint32_t values[4] = {0};
    for (size_t i = 0; i < n; ++i)
    {
        unsigned key = f[i].key;
        if (key >= 128)
            continue;
        if (!key || key > 4 || ((seen >> (key - 1)) & 1u))
            return AR_INVALID;
        seen |= 1u << (key - 1);
        values[key - 1] = f[i].value;
    }
    if (seen != 15 || values[0] != 1 || values[1] > AR_CAP || values[2] > 2 ||
        !values[3] || values[3] > 1000 || (values[2] == 2 && values[1] < 2))
        return AR_RANGE;
    struct ar_config result = {
        .timeout_ms = values[3], .limit = values[1], .policy = values[2]};
    if (AR_MUTANT == 5)
        result.limit = AR_CAP;
    *out = result;
    return AR_OK;
}

int ar_identity(struct ar_config c, unsigned char out[8])
{
    if (!out || c.limit > AR_CAP || c.policy > 2 || !c.timeout_ms ||
        c.timeout_ms > 1000 || (c.policy == 2 && c.limit < 2))
        return AR_INVALID;
    out[0] = 1;
    out[1] = (unsigned char)c.limit;
    out[2] = (unsigned char)c.policy;
    out[3] = 0;
    for (unsigned i = 0; i < 4; ++i)
        out[4 + i] = (unsigned char)(c.timeout_ms >> (24 - 8 * i));
    return AR_OK;
}
