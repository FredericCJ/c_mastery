#include "bytes.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8 && sizeof(struct aw_record) == 8,
               "selected record and byte model");

enum ab_status ab_write(uint64_t old, unsigned width, uint32_t value,
                        uint64_t *out)
{
    if (!out || (width != 1 && width != 2 && width != 4) ||
        (width == 1 && value > 255) || (width == 2 && value > 65535))
        return AB_INVALID;
    uint64_t mask = width == 1   ? UINT64_C(255)
                    : width == 2 ? UINT64_C(65535)
                                 : UINT64_C(0xffffffff);
#ifdef AB_MUTANT_PARTIAL
    old = 0;
#endif
    *out = width == 4 ? value : (old & ~mask) | value;
    return AB_OK;
}

enum ab_status ab_word(const unsigned char *p, size_t n, unsigned big,
                       uint32_t *out)
{
    if (!out || big > 1 || (!p && n))
        return AB_INVALID;
    if (n < 4)
        return AB_TRUNCATED;
#ifdef AB_MUTANT_ENDIAN
    big = 1 - big;
#endif
    uint32_t v = 0;
    for (unsigned i = 0; i < 4; ++i)
        v = (v << 8) | p[big ? i : 3 - i];
    *out = v;
    return AB_OK;
}

static enum ab_status offset(uint64_t base, int64_t delta, uint64_t *out)
{
    if (delta < 0)
    {
        uint64_t magnitude = (uint64_t)(-(delta + 1)) + 1;
        if (base < magnitude)
            return AB_RANGE;
        *out = base - magnitude;
    }
    else
    {
        if (base > UINT64_MAX - (uint64_t)delta)
            return AB_RANGE;
        *out = base + (uint64_t)delta;
    }
    return AB_OK;
}

enum ab_status ab_address(uint64_t base, uint64_t index, unsigned scale,
                          int64_t disp, unsigned width, uint64_t extent,
                          uint64_t *start, uint64_t *end)
{
    if (!start || !end || start == end ||
        (scale != 1 && scale != 2 && scale != 4 && scale != 8) || width == 0 ||
        width > 8)
        return AB_INVALID;
    unsigned shift = scale == 8 ? 3 : scale == 4 ? 2 : scale == 2 ? 1 : 0;
    if (index > (UINT64_MAX >> shift))
        return AB_RANGE;
    uint64_t pos;
    if (offset(index << shift, disp, &pos) != AB_OK || pos > extent)
        return AB_RANGE;
#ifndef AB_MUTANT_EXTENT
    if (width > extent - pos)
        return AB_RANGE;
#endif
    if (base > UINT64_MAX - pos || base + pos > UINT64_MAX - width)
        return AB_RANGE;
    *start = base + pos;
    *end = *start + width;
    return AB_OK;
}

static int32_t signed32(uint32_t u)
{
    return u <= INT32_MAX ? (int32_t)u : -(int32_t)(UINT32_MAX - u) - 1;
}

enum ab_status ab_record_decode(const unsigned char *p, size_t n, unsigned big,
                                struct aw_record *out)
{
    if (!out || big > 1 || (!p && n))
        return AB_INVALID;
    if (n != 8)
        return AB_TRUNCATED;
    uint32_t a, b;
    if (ab_word(p, 4, big, &a) != AB_OK || ab_word(p + 4, 4, big, &b) != AB_OK)
        return AB_INVALID;
    struct aw_record value = {signed32(a), signed32(b)};
    *out = value;
    return AB_OK;
}

enum ab_status ab_record_encode(const struct aw_record *p, unsigned big,
                                unsigned char *out, size_t n)
{
    if (!p || !out || big > 1)
        return AB_INVALID;
    if (n != 8)
        return AB_TRUNCATED;
    uint32_t values[2] = {(uint32_t)p->id, (uint32_t)p->rank};
    for (unsigned k = 0; k < 2; ++k)
        for (unsigned i = 0; i < 4; ++i)
            out[k * 4 + (big ? 3 - i : i)] =
                (unsigned char)(values[k] >> (i * 8));
    return AB_OK;
}

enum ab_status ab_record_copy(struct aw_record *dst,
                              const struct aw_record *src)
{
    if (!dst || !src)
        return AB_INVALID;
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < sizeof(*src); ++i)
        d[i] = s[i];
    return AB_OK;
}

unsigned ab_thumb_length(uint16_t h)
{
    return (h >> 11) >= 29 ? 4 : 2;
}

enum ab_status ab_x86(const unsigned char *p, size_t n, uint64_t pc,
                      unsigned extension, struct ab_instruction *out)
{
    if (!out || extension > 1 || (!p && n))
        return AB_INVALID;
    if (!n)
        return AB_TRUNCATED;
    struct ab_instruction v = {0};
    if (p[0] == 0x48)
    {
        if (n < 2)
            return AB_TRUNCATED;
        if (p[1] != 0x3d)
            return AB_UNSUPPORTED;
        if (n < 6)
            return AB_TRUNCATED;
        uint32_t immediate;
        if (ab_word(p + 2, 4, 0, &immediate) != AB_OK)
            return AB_INVALID;
        v.kind = AB_CMP;
        v.length = 6;
        v.operand_bits = 64;
        v.immediate = immediate;
        v.signed_immediate = immediate <= INT32_MAX
                                 ? (int64_t)immediate
                                 : (int64_t)immediate - INT64_C(4294967296);
    }
    else if (p[0] == 0x77)
    {
        if (n < 2)
            return AB_TRUNCATED;
        v.kind = AB_BRANCH;
        v.length = 2;
        v.condition = 7;
        v.signed_immediate = p[1] < 128 ? p[1] : (int)p[1] - 256;
        if (pc > UINT64_MAX - 2)
            return AB_RANGE;
#ifdef AB_MUTANT_PC
        uint64_t next = pc;
#else
        uint64_t next = pc + 2;
#endif
        if (offset(next, v.signed_immediate, &v.target) != AB_OK)
            return AB_RANGE;
    }
    else if (p[0] == 0x3c)
    {
#ifndef AB_MUTANT_EXTENSION
        if (!extension)
            return AB_UNSUPPORTED;
#endif
        if (n < 2)
            return AB_TRUNCATED;
        v.kind = AB_CMP;
        v.length = 2;
        v.operand_bits = 8;
        v.immediate = p[1];
        v.signed_immediate = p[1];
    }
    else
        return AB_UNSUPPORTED;
    *out = v;
    return AB_OK;
}

enum ab_status ab_thumb(const unsigned char *p, size_t n, uint32_t pc,
                        unsigned extension, struct ab_instruction *out)
{
    if (!out || extension > 1 || (!p && n) || (pc & 1))
        return AB_INVALID;
    if (n < 2)
        return AB_TRUNCATED;
    uint16_t h = (uint16_t)((unsigned)p[0] | ((unsigned)p[1] << 8));
    unsigned len = ab_thumb_length(h);
    if (n < len)
        return AB_TRUNCATED;
    if (len == 4)
        return AB_UNSUPPORTED;
    struct ab_instruction v = {0};
    v.length = 2;
    if ((h & 0xf800) == 0x2800)
    {
        v.kind = AB_CMP;
        v.operand_bits = 32;
        v.reg = (h >> 8) & 7;
        v.immediate = h & 255;
        v.signed_immediate = h & 255;
    }
    else if (extension && (h & 0xf000) == 0xd000 && ((h >> 8) & 15) < 14)
    {
        v.kind = AB_BRANCH;
        v.condition = (h >> 8) & 15;
        int delta = (h & 255) < 128 ? (h & 255) : (int)(h & 255) - 256;
        v.signed_immediate = delta * 2;
        if (pc > UINT32_MAX - 4 ||
            offset((uint64_t)pc + 4, v.signed_immediate, &v.target) != AB_OK ||
            v.target > UINT32_MAX)
            return AB_RANGE;
    }
    else
        return AB_UNSUPPORTED;
    *out = v;
    return AB_OK;
}
