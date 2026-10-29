#include "representation.h"
#ifndef RV_MUTANT
#define RV_MUTANT 0
#endif
static int valid(struct rv_value v)
{
    return v.tag <= RV_Q16 &&
           (v.tag != RV_BOOL || (v.payload == 0 || v.payload == 1)) &&
           (v.tag != RV_REF ||
            (v.payload >= 0 && v.payload < (int32_t)CVM_HEAP && v.generation));
}

int rv_import(struct cvm_value c, struct rv_value *out)
{
    if (!out)
        return RV_ARGUMENT;
    struct rv_value v = {0, 0, 0};
    switch (c.tag)
    {
        case CVM_NIL:
            v.tag = RV_NIL;
            break;
        case CVM_INT:
            v.tag = RV_INT;
            v.payload = c.integer;
            break;
        case CVM_BOOL:
            v.tag = RV_BOOL;
            v.payload = c.integer;
            break;
        case CVM_REF:
            v.tag = RV_REF;
            v.payload = c.integer;
            v.generation = c.generation;
            break;
        default:
            return RV_TYPE;
    }
    if (!valid(v))
        return RV_TYPE;
    *out = v;
    return RV_OK;
}

int rv_equal(struct rv_value a, struct rv_value b, int *out)
{
    if (!out)
        return RV_ARGUMENT;
    if (!valid(a) || !valid(b))
        return RV_TYPE;
    if (RV_MUTANT == 1)
    {
        *out = a.payload == b.payload;
        return RV_OK;
    }
    *out =
        a.tag == b.tag && (a.tag == RV_NIL ||
                           (a.payload == b.payload &&
                            (a.tag != RV_REF || a.generation == b.generation)));
    return RV_OK;
}

int rv_arithmetic(unsigned op, struct rv_value a, struct rv_value b,
                  struct rv_value *out)
{
    if (!out || op > RV_DIV)
        return RV_ARGUMENT;
    if (!valid(a) || !valid(b))
        return RV_TYPE;
    if (RV_MUTANT == 2)
    {
        if (a.tag == RV_BOOL)
            a.tag = RV_INT;
        if (b.tag == RV_BOOL)
            b.tag = RV_INT;
    }
    if ((a.tag != RV_INT && a.tag != RV_Q16) ||
        (b.tag != RV_INT && b.tag != RV_Q16))
        return RV_TYPE;
    unsigned fixed = a.tag == RV_Q16 || b.tag == RV_Q16;
    int64_t x = a.payload, y = b.payload, z;
    if (fixed)
    {
        if (a.tag == RV_INT)
            x *= INT64_C(65536);
        if (b.tag == RV_INT)
            y *= INT64_C(65536);
        if (x < INT32_MIN || x > INT32_MAX || y < INT32_MIN || y > INT32_MAX)
            return RV_RANGE;
    }
    switch (op)
    {
        case RV_ADD:
            z = x + y;
            break;
        case RV_SUB:
            z = x - y;
            break;
        case RV_MUL:
            z = x * y;
            if (fixed)
                z /= 65536;
            break;
        default:
            if (!y)
                return RV_RANGE;
            z = (fixed ? x * 65536 : x) / y;
            break;
    }
    if (RV_MUTANT == 3 && fixed && op == RV_MUL && x * y < 0 && x * y % 65536)
        z--;
    if (z < INT32_MIN || z > INT32_MAX)
        return RV_RANGE;
    *out = (struct rv_value){fixed ? RV_Q16 : RV_INT, (int32_t)z, 0};
    return RV_OK;
}

static unsigned u16(const uint8_t *b)
{
    return (unsigned)b[0] | ((unsigned)b[1] << 8);
}

static void p16(uint8_t *b, unsigned v)
{
    b[0] = (uint8_t)v;
    b[1] = (uint8_t)(v >> 8);
}

static int text(const uint8_t *b, unsigned n)
{
    if (!n || n > 31)
        return 0;
    for (unsigned i = 0; i < n; i++)
        if (b[i] < 32 || b[i] > 126)
            return 0;
    return 1;
}

int rv_decode(const uint8_t *b, size_t n, struct rv_image *out)
{
    if (!b || !out)
        return RV_ARGUMENT;
    if (n < 9 || n > RV_IMAGE_MAX)
        return RV_FORMAT;
    if (b[0] != 'R' || b[1] != 'V' || b[2] != 'E' || b[3] != '2')
        return RV_FORMAT;
    if (b[4] != 1)
        return RV_VERSION;
    if (b[5] || b[8] > 4)
        return RV_FORMAT;
    size_t base = u16(b + 6), pos = 9;
    if (base > n - pos)
        return RV_FORMAT;
    struct rv_image result = {0};
    if (cvm_decode(b + pos, base, &result.program) != CVM_OK)
        return RV_FORMAT;
    pos += base;
    for (unsigned i = 0; i < b[8]; i++)
    {
        if (n - pos < 4)
            return RV_FORMAT;
        unsigned type = b[pos], flags = b[pos + 1], len = u16(b + pos + 2);
        pos += 4;
        if (flags > 1 || len > 64 || len > n - pos)
            return RV_FORMAT;
        if (type == 1)
        {
            if (len != 32 || result.has_digest)
                return RV_FORMAT;
            for (unsigned k = 0; k < 32; k++)
                result.digest[k] = b[pos + k];
            result.has_digest = 1;
        }
        else if (type == 2)
        {
            if (result.name_length || !text(b + pos, len))
                return RV_FORMAT;
            for (unsigned k = 0; k < len; k++)
                result.name[k] = b[pos + k];
            result.name_length = (uint8_t)len;
        }
        else
        {
            if (flags && RV_MUTANT != 4)
                return RV_REQUIRED;
            result.skipped++;
        }
        pos += len;
        if (RV_MUTANT == 5)
            *out = result;
    }
    if (pos != n && RV_MUTANT != 6)
        return RV_FORMAT;
    *out = result;
    return RV_OK;
}

int rv_encode(const struct rv_image *in, uint8_t *b, size_t cap,
              size_t *written)
{
    if (!in || !b || !written)
        return RV_ARGUMENT;
    if (in->has_digest > 1 || in->name_length > 31 || in->skipped ||
        (in->name_length && !text(in->name, in->name_length)))
        return RV_FORMAT;
    uint8_t temp[RV_IMAGE_MAX] = {'R', 'V', 'E', '2', 1, 0, 0, 0, 0};
    size_t base = 0, pos;
    if (cvm_encode(&in->program, temp + 9, sizeof temp - 9, &base) != CVM_OK)
        return RV_FORMAT;
    p16(temp + 6, (unsigned)base);
    pos = 9 + base;
    if (in->has_digest)
    {
        temp[8]++;
        temp[pos] = 1;
        temp[pos + 1] = 0;
        p16(temp + pos + 2, 32);
        pos += 4;
        for (unsigned k = 0; k < 32; k++)
            temp[pos++] = in->digest[k];
    }
    if (in->name_length)
    {
        temp[8]++;
        temp[pos] = 2;
        temp[pos + 1] = 0;
        p16(temp + pos + 2, in->name_length);
        pos += 4;
        for (unsigned k = 0; k < in->name_length; k++)
            temp[pos++] = in->name[k];
    }
    if (cap < pos)
        return RV_RANGE;
    for (size_t i = 0; i < pos; i++)
        b[i] = temp[i];
    *written = pos;
    return RV_OK;
}
