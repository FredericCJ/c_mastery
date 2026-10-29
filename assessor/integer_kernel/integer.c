#include "integer.h"
#include <limits.h>
#ifndef FI_MUTANT
#define FI_MUTANT 0
#endif
static int valid(struct fi_range r)
{
    return r.low <= 0 && r.high >= 0;
}

static int admits(struct fi_range r, int a)
{
    return a >= r.low && a <= r.high;
}

enum fi_status fi_binary(struct fi_range r, enum fi_operation op, int a, int b,
                         int *out)
{
    if (!out || !valid(r) || (unsigned)op > FI_MULTIPLY)
    {
        return FI_INVALID;
    }
    if (!admits(r, a) || !admits(r, b))
    {
        return FI_RANGE;
    }
    if (FI_MUTANT == 1)
    {
        *out = 0;
    }
    if (op == FI_ADD)
    {
        if ((b > 0 && a > r.high - b) || (b < 0 && a < r.low - b))
        {
            return FI_RANGE;
        }
        *out = a + b;
    }
    else if (op == FI_SUBTRACT)
    {
        if ((b < 0 && a > r.high + b) || (b > 0 && a < r.low + b))
        {
            return FI_RANGE;
        }
        *out = a - b;
    }
    else
    {
        if (a > 0 ? (b > 0 ? a > r.high / b : b < r.low / a)
                  : (b > 0 ? a < r.low / b : a != 0 && b < r.high / a))
        {
            return FI_RANGE;
        }
        *out = a * b;
    }
    return FI_OK;
}

enum fi_status fi_saturate(struct fi_range r, enum fi_operation op, int a,
                           int b, int *out, unsigned *clipped)
{
    if (!out || !clipped || (void *)out == (void *)clipped || !valid(r) ||
        (unsigned)op > FI_MULTIPLY)
    {
        return FI_INVALID;
    }
    if (!admits(r, a) || !admits(r, b))
    {
        return FI_RANGE;
    }
    int value = 0;
    enum fi_status status = fi_binary(r, op, a, b, &value);
    if (status == FI_RANGE)
    {
        int positive = op == FI_ADD        ? b > 0
                       : op == FI_SUBTRACT ? b < 0
                                           : (a < 0) == (b < 0);
        *out = positive ? r.high : r.low;
        *clipped = FI_MUTANT == 5 ? 0 : 1;
    }
    else
    {
        *out = value;
        *clipped = 0;
    }
    return FI_OK;
}

enum fi_status fi_divide(struct fi_range range, enum fi_division policy, int a,
                         int b, int *q, int *rem)
{
    if (!q || !rem || q == rem || !valid(range) ||
        (unsigned)policy > FI_EUCLIDEAN)
    {
        return FI_INVALID;
    }
    if (!admits(range, a) || !admits(range, b))
    {
        return FI_RANGE;
    }
    if (b == 0 || (a == INT_MIN && b == -1 && INT_MIN < -INT_MAX) ||
        (a == range.low && b == -1 && range.low < -range.high))
    {
        return FI_RANGE;
    }
    int quotient = a / b, remainder = a % b;
    if (policy == FI_FLOOR && remainder && (remainder < 0) != (b < 0))
    {
        --quotient;
        remainder += b;
    }
    else if (policy == FI_EUCLIDEAN && remainder < 0)
    {
        if (b > 0)
        {
            --quotient;
            remainder += b;
        }
        else
        {
            ++quotient;
            remainder -= b;
        }
    }
    if (!admits(range, quotient) || !admits(range, remainder))
    {
        return FI_RANGE;
    }
    if (FI_MUTANT == 2 && remainder < 0)
    {
        remainder = -remainder;
    }
    *q = quotient;
    *rem = remainder;
    return FI_OK;
}

enum fi_status fi_mod_positive(int a, int m, int *out)
{
    if (!out || m <= 0)
    {
        return FI_INVALID;
    }
    int r = a % m;
    *out = r < 0 ? r + m : r;
    return FI_OK;
}

enum fi_status fi_to_size(intmax_t value, size_t ceiling, size_t *out)
{
    if (!out)
    {
        return FI_INVALID;
    }
    if (FI_MUTANT == 3 && value < 0)
    {
        *out = 0;
        return FI_RANGE;
    }
    if (value < 0 || (uintmax_t)value > (uintmax_t)ceiling)
    {
        return FI_RANGE;
    }
    *out = (size_t)value;
    return FI_OK;
}

enum fi_status fi_extent(size_t prefix, size_t count, size_t width,
                         size_t limit, size_t *out)
{
    if (!out)
    {
        return FI_INVALID;
    }
    if (FI_MUTANT == 4 && !width)
    {
        *out = prefix;
        return FI_OK;
    }
    if (prefix > limit || (width && count > (limit - prefix) / width))
    {
        return FI_RANGE;
    }
    *out = prefix + count * width;
    return FI_OK;
}

enum fi_status fi_layout(size_t prefix, intmax_t count, size_t width,
                         size_t limit, size_t *out)
{
    if (!out)
    {
        return FI_INVALID;
    }
    size_t n;
    enum fi_status status = fi_to_size(count, SIZE_MAX, &n);
    return status == FI_OK ? fi_extent(prefix, n, width, limit, out) : status;
}

enum fi_status fi_window(uintmax_t base, uintmax_t position, size_t extent,
                         size_t *offset)
{
    if (!offset)
    {
        return FI_INVALID;
    }
    if (position < base || position - base > (uintmax_t)extent)
    {
        return FI_RANGE;
    }
    *offset = (size_t)(position - base);
    return FI_OK;
}

enum fi_status fi_encode_cells(uint_least32_t wire, unsigned int out[4])
{
    if (!out)
    {
        return FI_INVALID;
    }
    if (wire > UINT32_C(0xffffffff))
    {
        return FI_RANGE;
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        out[i] = (unsigned)((wire >> (24 - 8 * i)) & 255u);
    }
    return FI_OK;
}

enum fi_status fi_decode_cells(const unsigned int cells[4], uint_least32_t *out)
{
    if (!cells || !out)
    {
        return FI_INVALID;
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        if (cells[i] > 255u)
        {
            return FI_RANGE;
        }
    }
    uint_least32_t value = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        value = (value << 8) | cells[i];
    }
    *out = value;
    return FI_OK;
}

enum fi_status fi_encode_octets(uint_least32_t wire, unsigned char out[4])
{
    if (!out)
    {
        return FI_INVALID;
    }
    if (CHAR_BIT != 8)
    {
        return FI_UNSUPPORTED;
    }
    unsigned int cells[4];
    enum fi_status status = fi_encode_cells(wire, cells);
    if (status != FI_OK)
    {
        return status;
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        out[i] = (unsigned char)cells[i];
    }
    return FI_OK;
}
