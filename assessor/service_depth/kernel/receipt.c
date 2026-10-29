#include "receipt.h"

static uint32_t rd(const unsigned char *b)
{
    return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 |
           (uint32_t)b[3] << 24;
}

static void wr(unsigned char *b, uint32_t n)
{
    for (unsigned i = 0; i < 4; i++)
    {
        b[i] = (unsigned char)(n >> (8 * i));
    }
}

static int32_t signed32(uint32_t n)
{
    return n <= INT32_MAX ? (int32_t)n
                          : INT32_MIN + (int32_t)(n - UINT32_C(0x80000000));
}

static int get(store *s, const unsigned char *key, size_t n, unsigned char *v,
               size_t length)
{
    size_t got = 0;
    int r = store_get(s, key, n, v, length, &got);
    return r == STORE_OK && got == length ? SDR_OK : SDR_STORAGE;
}

static int commit(store *s)
{
    int r = store_commit(s);
    return r == STORE_OK
               ? SDR_OK
               : (r == STORE_INDETERMINATE ? SDR_UNCERTAIN : SDR_STORAGE);
}

int sdr_initialize(store *s)
{
    if (!s || store_begin(s) != STORE_OK)
    {
        return SDR_STORAGE;
    }
    unsigned char zero[4] = {0}, meta[12] = {0};
    if (store_put(s, "effect", 6, zero, 4) != STORE_OK)
    {
        goto fail;
    }
    for (unsigned client = 1; client <= 2; client++)
    {
        unsigned char key[2] = {'m', (unsigned char)client};
        wr(meta, 1);
        if (store_put(s, key, 2, meta, 12) != STORE_OK)
        {
            goto fail;
        }
    }
    return commit(s);
fail:
    store_abort(s);
    return SDR_STORAGE;
}

int sdr_value(store *s, int32_t *out)
{
    unsigned char value[4];
    if (!s || !out)
    {
        return SDR_INVALID;
    }
    if (get(s, (const unsigned char *)"effect", 6, value, 4))
    {
        return SDR_STORAGE;
    }
    *out = signed32(rd(value));
    return SDR_OK;
}

int sdr_apply(store *s, uint32_t client, uint32_t epoch, uint32_t id,
              int32_t delta, int32_t *out)
{
    if (!s || !out || client < 1 || client > 2 || !epoch || !id)
    {
        return SDR_INVALID;
    }
    unsigned char mk[2] = {'m', (unsigned char)client}, meta[12];
    if (get(s, mk, 2, meta, 12))
    {
        return SDR_STORAGE;
    }
    uint32_t current = rd(meta), high = rd(meta + 4), next = rd(meta + 8);
    (void)high;
    if (epoch != current)
    {
        return SDR_STALE;
    }
    if (next >= 3)
    {
        return SDR_STORAGE;
    }
    for (unsigned slot = 0; slot < 3; slot++)
    {
        unsigned char key[3] = {'r', (unsigned char)client,
                                (unsigned char)slot},
                      record[12];
        size_t n = 0;
        int r = store_get(s, key, 3, record, sizeof record, &n);
        if (r == STORE_NOT_FOUND)
        {
            continue;
        }
        if (r != STORE_OK || n != 12)
        {
            return SDR_STORAGE;
        }
        if (rd(record) == id)
        {
#ifndef SDK_MUTANT_PAYLOAD_BINDING
            if (signed32(rd(record + 4)) != delta)
            {
                return SDR_CONFLICT;
            }
#endif
#ifndef SDK_MUTANT_REAPPLY
            *out = signed32(rd(record + 8));
            return SDR_OK;
#endif
        }
    }
#ifndef SDK_MUTANT_REAPPLY
    if (id <= high)
    {
        return SDR_STALE;
    }
#endif
    int32_t previous;
    if (sdr_value(s, &previous))
    {
        return SDR_STORAGE;
    }
    int64_t sum = (int64_t)previous + delta;
    if (sum < INT32_MIN || sum > INT32_MAX)
    {
        return SDR_OVERFLOW;
    }
    int32_t value = (int32_t)sum;
    unsigned char effect[4], record[12],
        key[3] = {'r', (unsigned char)client, (unsigned char)next};
    wr(effect, (uint32_t)value);
    wr(record, id);
    wr(record + 4, (uint32_t)delta);
    wr(record + 8, (uint32_t)value);
    wr(meta + 4, id);
    wr(meta + 8, (next + 1) % 3);
    if (store_begin(s) != STORE_OK)
    {
        return SDR_STORAGE;
    }
    if (store_put(s, "effect", 6, effect, 4) != STORE_OK)
    {
        goto rollback;
    }
#ifdef SDK_MUTANT_SPLIT_RECEIPT
    if (commit(s) != SDR_OK)
    {
        return SDR_UNCERTAIN;
    }
    if (store_begin(s) != STORE_OK)
    {
        return SDR_STORAGE;
    }
#endif
    if (store_put(s, key, 3, record, 12) != STORE_OK ||
        store_put(s, mk, 2, meta, 12) != STORE_OK)
    {
        goto rollback;
    }
    int r = commit(s);
    if (r)
    {
        return r;
    }
    *out = value;
    return SDR_OK;
rollback:
    store_abort(s);
    return SDR_STORAGE;
}
