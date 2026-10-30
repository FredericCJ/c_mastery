/*
 * Reference implementation of the integer contracts in integer.h.
 *
 * Every arithmetic guard is written with operations that cannot themselves
 * overflow, and never with a wider type (SEI CERT INT32-C), so the same code
 * is correct for an int of any width. A range is a closed interval that must
 * contain zero; every operand must lie inside it. Each function validates its
 * arguments first, then computes, and writes its outputs only on success
 * (failure atomicity): on any non-FI_OK return the caller's outputs are
 * untouched.
 *
 * FI_MUTANT selects a deliberate defect for the mutation campaign. 0 builds
 * the correct reference.
 */
#include "integer.h"
#include <limits.h>
#include <stdbool.h>

#ifndef FI_MUTANT
#define FI_MUTANT 0
#endif

#define OCTETS_PER_WIRE 4u
#define BITS_PER_OCTET 8u
#define OCTET_MASK 0xffu

/* A range is usable only if it contains zero, so that x - x and 0 * x are
 * always representable and the guards below cannot overflow. */
static bool range_is_valid(struct fi_range range)
{
    return (range.low <= 0) && (range.high >= 0);
}

static bool range_admits(struct fi_range range, int value)
{
    return (value >= range.low) && (value <= range.high);
}

/* The enumerations arrive from the caller as plain integers. Converting to
 * unsigned makes a negative value huge, so one comparison rejects both ends. */
static bool operation_is_known(enum fi_operation op)
{
    return (unsigned)op <= (unsigned)FI_MULTIPLY;
}

static bool division_policy_is_known(enum fi_division policy)
{
    return (unsigned)policy <= (unsigned)FI_EUCLIDEAN;
}

/*
 * The three predicates below answer "does the exact result leave the range?"
 * for operands already known to be inside it. Add and subtract compare an
 * operand with a bound adjusted by the other operand; the adjustment is
 * chosen by the sign of that operand so it never overflows int. Multiply
 * adjusts nothing: it compares an operand with a quotient of a bound.
 */
static bool add_leaves_range(struct fi_range range, int a, int b)
{
    if (b > 0)
    {
        return a > range.high - b;
    }
    if (b < 0)
    {
        return a < range.low - b;
    }
    return false;
}

static bool subtract_leaves_range(struct fi_range range, int a, int b)
{
    if (b < 0)
    {
        return a > range.high + b;
    }
    if (b > 0)
    {
        return a < range.low + b;
    }
    return false;
}

/* Split by operand signs so each quotient bound is taken with a divisor of
 * known sign: the divisions below never divide by zero and never compute
 * INT_MIN / -1. */
static bool multiply_leaves_range(struct fi_range range, int a, int b)
{
    if (a > 0)
    {
        if (b > 0)
        {
            return a > range.high / b;
        }
        return b < range.low / a;
    }
    if (b > 0)
    {
        return a < range.low / b;
    }
    if (a == 0)
    {
        return false;
    }
    return b < range.high / a;
}

static bool operation_leaves_range(struct fi_range range, enum fi_operation op,
                                   int a, int b)
{
    if (op == FI_ADD)
    {
        return add_leaves_range(range, a, b);
    }
    if (op == FI_SUBTRACT)
    {
        return subtract_leaves_range(range, a, b);
    }
    return multiply_leaves_range(range, a, b);
}

/* Precondition: operation_leaves_range() is false, so the native operator
 * below cannot overflow. */
static int operation_value(enum fi_operation op, int a, int b)
{
    if (op == FI_ADD)
    {
        return a + b;
    }
    if (op == FI_SUBTRACT)
    {
        return a - b;
    }
    return a * b;
}

enum fi_status fi_binary(struct fi_range range, enum fi_operation op, int a,
                         int b, int *out)
{
    if ((out == NULL) || !range_is_valid(range) || !operation_is_known(op))
    {
        return FI_INVALID;
    }
    if (!range_admits(range, a) || !range_admits(range, b))
    {
        return FI_RANGE;
    }
    if (FI_MUTANT == 1)
    {
        *out = 0; /* mutant: breaks failure atomicity on FI_RANGE below */
    }
    if (operation_leaves_range(range, op, a, b))
    {
        return FI_RANGE;
    }
    *out = operation_value(op, a, b);
    return FI_OK;
}

/* True when an out-of-range exact result lies above range.high. */
static bool overflow_is_upward(enum fi_operation op, int a, int b)
{
    if (op == FI_ADD)
    {
        return b > 0;
    }
    if (op == FI_SUBTRACT)
    {
        return b < 0;
    }
    return (a < 0) == (b < 0); /* equal signs: positive product */
}

enum fi_status fi_saturate(struct fi_range range, enum fi_operation op, int a,
                           int b, int *out, unsigned *clipped)
{
    /* The two outputs have different types; comparing as void * is the only
     * way to reject a caller that passes the same storage for both. */
    bool outputs_alias = (void *)out == (void *)clipped;

    if ((out == NULL) || (clipped == NULL) || outputs_alias ||
        !range_is_valid(range) || !operation_is_known(op))
    {
        return FI_INVALID;
    }
    if (!range_admits(range, a) || !range_admits(range, b))
    {
        return FI_RANGE;
    }

    int value = 0;
    enum fi_status status = fi_binary(range, op, a, b, &value);
    if (status == FI_RANGE)
    {
        *out = overflow_is_upward(op, a, b) ? range.high : range.low;
        *clipped = (FI_MUTANT == 5) ? 0u : 1u;
    }
    else
    {
        *out = value;
        *clipped = 0u;
    }
    return FI_OK;
}

/* A truncating quotient is unrepresentable when the divisor is zero or the
 * single overflowing case "most negative / -1" occurs. The native check is
 * only needed when INT_MIN is not -INT_MAX (two's complement); the range
 * check covers narrower ranges such as [-128, 127]. */
static bool quotient_is_unrepresentable(struct fi_range range, int a, int b)
{
    if (b == 0)
    {
        return true;
    }
    if ((a == INT_MIN) && (b == -1) && (INT_MIN < -INT_MAX))
    {
        return true;
    }
    return (a == range.low) && (b == -1) && (range.low < -range.high);
}

/*
 * Convert the truncating quotient and remainder (C's / and %) to another
 * convention in place. Truncation: remainder has the sign of the dividend.
 * Floor: remainder has the sign of the divisor. Euclidean: remainder is never
 * negative. Each adjustment keeps a == quotient * b + remainder, and moves the
 * remainder toward the divisor's side so it cannot overflow.
 */
static void apply_division_policy(enum fi_division policy, int b, int *quotient,
                                  int *remainder)
{
    if (policy == FI_FLOOR)
    {
        bool signs_differ = (*remainder < 0) != (b < 0);
        if ((*remainder != 0) && signs_differ)
        {
            --*quotient;
            *remainder += b;
        }
    }
    else if ((policy == FI_EUCLIDEAN) && (*remainder < 0))
    {
        if (b > 0)
        {
            --*quotient;
            *remainder += b;
        }
        else
        {
            ++*quotient;
            *remainder -= b;
        }
    }
    else
    {
        /* FI_TRUNCATE, or a non-negative Euclidean remainder: no change. */
    }
}

enum fi_status fi_divide(struct fi_range range, enum fi_division policy, int a,
                         int b, int *quotient_out, int *remainder_out)
{
    if ((quotient_out == NULL) || (remainder_out == NULL) ||
        (quotient_out == remainder_out) || !range_is_valid(range) ||
        !division_policy_is_known(policy))
    {
        return FI_INVALID;
    }
    if (!range_admits(range, a) || !range_admits(range, b))
    {
        return FI_RANGE;
    }
    if (quotient_is_unrepresentable(range, a, b))
    {
        return FI_RANGE;
    }

    int quotient = a / b;
    int remainder = a % b;
    apply_division_policy(policy, b, &quotient, &remainder);
    if (!range_admits(range, quotient) || !range_admits(range, remainder))
    {
        return FI_RANGE;
    }
    if ((FI_MUTANT == 2) && (remainder < 0))
    {
        remainder = -remainder; /* mutant: wrong sign convention */
    }
    *quotient_out = quotient;
    *remainder_out = remainder;
    return FI_OK;
}

enum fi_status fi_mod_positive(int a, int modulus, int *out)
{
    if ((out == NULL) || (modulus <= 0))
    {
        return FI_INVALID;
    }
    /* remainder lies in (-modulus, modulus), so adding modulus to a negative
     * remainder lands in (0, modulus) and cannot overflow. */
    int remainder = a % modulus;
    *out = (remainder < 0) ? (remainder + modulus) : remainder;
    return FI_OK;
}

enum fi_status fi_to_size(intmax_t value, size_t ceiling, size_t *out)
{
    if (out == NULL)
    {
        return FI_INVALID;
    }
    if ((FI_MUTANT == 3) && (value < 0))
    {
        *out = 0; /* mutant: writes an output on failure */
        return FI_RANGE;
    }
    /* Sign is rejected first; only then is value converted to uintmax_t,
     * where it is non-negative and therefore exact (CERT INT31-C). */
    if ((value < 0) || ((uintmax_t)value > (uintmax_t)ceiling))
    {
        return FI_RANGE;
    }
    *out = (size_t)value; /* value <= ceiling <= SIZE_MAX: exact */
    return FI_OK;
}

enum fi_status fi_extent(size_t prefix, size_t count, size_t width,
                         size_t limit, size_t *out)
{
    if (out == NULL)
    {
        return FI_INVALID;
    }
    if ((FI_MUTANT == 4) && (width == 0))
    {
        *out = prefix; /* mutant: skips the limit check for zero width */
        return FI_OK;
    }
    /* count * width <= limit - prefix is tested as a division so the product
     * is only formed once it is known not to wrap (CERT INT30-C). */
    if (prefix > limit)
    {
        return FI_RANGE;
    }
    if ((width != 0) && (count > (limit - prefix) / width))
    {
        return FI_RANGE;
    }
    *out = prefix + count * width;
    return FI_OK;
}

enum fi_status fi_layout(size_t prefix, intmax_t count, size_t width,
                         size_t limit, size_t *out)
{
    if (out == NULL)
    {
        return FI_INVALID;
    }

    size_t element_count = 0;
    enum fi_status status = fi_to_size(count, SIZE_MAX, &element_count);
    if (status != FI_OK)
    {
        return status;
    }
    return fi_extent(prefix, element_count, width, limit, out);
}

enum fi_status fi_window(uintmax_t logical_base, uintmax_t position,
                         size_t extent, size_t *offset)
{
    if (offset == NULL)
    {
        return FI_INVALID;
    }
    /* Positions are logical numbers, not pointers: subtraction happens only
     * after position >= logical_base, so it cannot wrap. */
    if ((position < logical_base) ||
        ((position - logical_base) > (uintmax_t)extent))
    {
        return FI_RANGE;
    }
    *offset = (size_t)(position - logical_base); /* <= extent: exact */
    return FI_OK;
}

/* Cell i of a big-endian wire word: octet 0 is the most significant. */
static unsigned octet_shift_bits(unsigned index)
{
    return (OCTETS_PER_WIRE - 1u - index) * BITS_PER_OCTET;
}

enum fi_status fi_encode_cells(uint_least32_t wire,
                               unsigned int out[OCTETS_PER_WIRE])
{
    if (out == NULL)
    {
        return FI_INVALID;
    }
    /* uint_least32_t may be wider than 32 bits; the wire word is not. */
    if (wire > UINT32_C(0xffffffff))
    {
        return FI_RANGE;
    }
    for (unsigned i = 0; i < OCTETS_PER_WIRE; ++i)
    {
        uint_least32_t octet = (wire >> octet_shift_bits(i)) & OCTET_MASK;
        out[i] = (unsigned int)octet; /* octet <= 255: exact */
    }
    return FI_OK;
}

enum fi_status fi_decode_cells(const unsigned int cells[OCTETS_PER_WIRE],
                               uint_least32_t *out)
{
    if ((cells == NULL) || (out == NULL))
    {
        return FI_INVALID;
    }
    /* Validate every cell before combining any: a cell above 255 would
     * overlap its neighbour and silently corrupt the word. */
    for (unsigned i = 0; i < OCTETS_PER_WIRE; ++i)
    {
        if (cells[i] > OCTET_MASK)
        {
            return FI_RANGE;
        }
    }

    uint_least32_t value = 0;
    for (unsigned i = 0; i < OCTETS_PER_WIRE; ++i)
    {
        value = (value << BITS_PER_OCTET) | cells[i];
    }
    *out = value;
    return FI_OK;
}

enum fi_status fi_encode_octets(uint_least32_t wire,
                                unsigned char out[OCTETS_PER_WIRE])
{
    if (out == NULL)
    {
        return FI_INVALID;
    }
    if (CHAR_BIT != 8)
    {
        return FI_UNSUPPORTED;
    }

    unsigned int cells[OCTETS_PER_WIRE];
    enum fi_status status = fi_encode_cells(wire, cells);
    if (status != FI_OK)
    {
        return status;
    }
    for (unsigned i = 0; i < OCTETS_PER_WIRE; ++i)
    {
        out[i] = (unsigned char)cells[i]; /* cells[i] <= 255, CHAR_BIT == 8 */
    }
    return FI_OK;
}
