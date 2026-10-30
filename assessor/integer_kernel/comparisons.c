/*
 * Comparison of the C17 reference with the compiler's checked-arithmetic
 * facility (C23 <stdckdint.h> or the GCC/Clang builtins), and with the earlier
 * foundation_depth conversion provider.
 *
 * ADD, SUB and MUL store the wrapped (or exact) result through their first
 * argument and return nonzero on overflow. The primitive stores a result even
 * when it overflows, so a public wrapper must stage into a temporary and
 * publish only on success. -DFI_BUILTIN_EARLY removes that staging in
 * builtin() on purpose; the campaign expects this program to then fail.
 */
#include "integer.h"
/* depth.h: actual unchanged prerequisite extent/conversion provider. */
#include "depth.h"
#include <limits.h>
#include <stdio.h>
#ifdef FI_C23
#include <stdckdint.h>
#if __STDC_VERSION__ < 202311L || __STDC_VERSION_STDCKDINT_H__ < 202311L
#error actual_C23_checked_arithmetic_capability_required
#endif
#define ADD(p, a, b) ckd_add(p, a, b)
#define SUB(p, a, b) ckd_sub(p, a, b)
#define MUL(p, a, b) ckd_mul(p, a, b)
#else
#if !defined(__has_builtin)
#error builtin_capability_probe_required
#endif
#if !__has_builtin(__builtin_add_overflow) || \
    !__has_builtin(__builtin_sub_overflow) || \
    !__has_builtin(__builtin_mul_overflow)
#error checked_builtin_capability_missing
#endif
#define ADD(p, a, b) __builtin_add_overflow(a, b, p)
#define SUB(p, a, b) __builtin_sub_overflow(a, b, p)
#define MUL(p, a, b) __builtin_mul_overflow(a, b, p)
#endif
#define CHECK(x)                                                      \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "FI_COMPARE_FAIL %d %s\n", __LINE__, #x); \
            return 1;                                                 \
        }                                                             \
    } while (0)

/* Dispatch on the operation enumeration; p may point at any integer type. */
#define CHECKED_OP(op, p, a, b)             \
    (((op) == FI_ADD)        ? ADD(p, a, b) \
     : ((op) == FI_SUBTRACT) ? SUB(p, a, b) \
                             : MUL(p, a, b))

/* Sentinels: an output that must stay untouched on failure. */
#define UNTOUCHED_INT 917
#define UNTOUCHED_SIZE 917u
#define UNTOUCHED_CHAR 91

/* The narrow destination experiment uses signed char with this range. */
#define NARROW_LOW (-128)
#define NARROW_HIGH 127

/* Bounds of the exhaustive small-size comparison with the earlier provider. */
#define SIZE_GRID 9u

/* Same contract as fi_binary, built on the compiler primitive. The result is
 * staged in a temporary and published only when it is exact and in range. */
static enum fi_status builtin(struct fi_range range, enum fi_operation op,
                              int a, int b, int *out)
{
    int temporary = 0;
    int overflow = CHECKED_OP(op, &temporary, a, b);
#ifdef FI_BUILTIN_EARLY
    *out = temporary; /* deliberate defect: publishes before checking */
#endif
    if (overflow || (temporary < range.low) || (temporary > range.high))
    {
        return FI_RANGE;
    }
    *out = temporary;
    return FI_OK;
}

/* Same primitive with a signed char destination: overflow is judged against
 * the destination type, not against int. */
static enum fi_status narrow(enum fi_operation op, int a, int b,
                             signed char *out)
{
    signed char temporary = 0;
    int overflow = CHECKED_OP(op, &temporary, a, b);
    if (overflow)
    {
        return FI_RANGE;
    }
    *out = temporary;
    return FI_OK;
}

/* Exact result of a (op) b; operands are small enough that int cannot wrap. */
static int exact_result(unsigned op, int a, int b)
{
    if (op == (unsigned)FI_ADD)
    {
        return a + b;
    }
    if (op == (unsigned)FI_SUBTRACT)
    {
        return a - b;
    }
    return a * b;
}

/* Every pair of signed-char values under every operation: the C17 reference,
 * the compiler primitive and the narrow-destination primitive must agree. */
static int compare_exhaustive_operations(unsigned long *operations)
{
    const struct fi_range range = {NARROW_LOW, NARROW_HIGH};

    for (int a = NARROW_LOW; a <= NARROW_HIGH; ++a)
    {
        for (int b = NARROW_LOW; b <= NARROW_HIGH; ++b)
        {
            for (unsigned op = 0; op < 3; ++op)
            {
                int exact = exact_result(op, a, b);
                enum fi_status expected =
                    (exact >= NARROW_LOW && exact <= NARROW_HIGH) ? FI_OK
                                                                  : FI_RANGE;
                int manual = UNTOUCHED_INT;
                int extension = UNTOUCHED_INT;
                signed char small = UNTOUCHED_CHAR;
                CHECK(fi_binary(range, (enum fi_operation)op, a, b, &manual) ==
                      expected);
                CHECK(builtin(range, (enum fi_operation)op, a, b, &extension) ==
                      expected);
                CHECK(manual == extension &&
                      extension == (expected == FI_OK ? exact : UNTOUCHED_INT));
                CHECK(narrow((enum fi_operation)op, a, b, &small) == expected &&
                      small == (expected == FI_OK ? exact : UNTOUCHED_CHAR));
                ++*operations;
            }
        }
    }
    return 0;
}

/* The primitives follow mathematical semantics, which ordinary conversions do
 * not: unsigned overflow wraps (and is reported), and a negative result is
 * kept exactly when the destination can hold it. */
static int check_primitive_semantics(void)
{
    unsigned int wrapped = 91;
    CHECK(ADD(&wrapped, UINT_MAX, 1u) && wrapped == 0);

    signed char negative = 91;
    CHECK(!MUL(&negative, -1, 1u) && negative == -1);

    /* Ordinary conversion converts -1 to unsigned first and wraps. */
    CHECK((-1 * 1u) == UINT_MAX);
    return 0;
}

/* Overflow at the native int edges is reported and leaves the output alone. */
static int check_native_edges(void)
{
    const struct fi_range native = {INT_MIN, INT_MAX};
    int edge = UNTOUCHED_INT;
    CHECK(builtin(native, FI_ADD, INT_MAX, 1, &edge) == FI_RANGE &&
          edge == UNTOUCHED_INT);
    CHECK(builtin(native, FI_SUBTRACT, INT_MIN, 1, &edge) == FI_RANGE &&
          edge == UNTOUCHED_INT);
    CHECK(builtin(native, FI_MULTIPLY, INT_MIN, -1, &edge) == FI_RANGE &&
          edge == UNTOUCHED_INT);
    return 0;
}

/* Composition: fi_extent and fi_to_size must agree with the earlier
 * fd_extent and fd_from_signed, the actual unchanged provider (not renamed
 * local substitutes). Every grid point has an exact result that fits in
 * size_t, so "exact <= limit" is the specification. */
static int compare_prior_extents(unsigned long *extents)
{
    for (size_t prefix = 0; prefix < SIZE_GRID; ++prefix)
    {
        for (size_t count = 0; count < SIZE_GRID; ++count)
        {
            for (size_t width = 0; width < SIZE_GRID; ++width)
            {
                for (size_t limit = 0; limit < SIZE_GRID; ++limit)
                {
                    size_t a = UNTOUCHED_SIZE;
                    size_t b = UNTOUCHED_SIZE;
                    size_t exact = prefix + count * width;
                    CHECK(fi_extent(prefix, count, width, limit, &a) ==
                          (exact <= limit ? FI_OK : FI_RANGE));
                    CHECK(fd_extent(prefix, count, width, limit, &b) ==
                          (exact <= limit ? FD_OK : FD_RANGE));
                    CHECK(a == b &&
                          a == (exact <= limit ? exact : UNTOUCHED_SIZE));
                    ++*extents;
                }
            }
        }
    }
    return 0;
}

static int compare_prior_conversion(void)
{
    for (intmax_t v = -9; v <= 9; ++v)
    {
        size_t a = UNTOUCHED_SIZE;
        size_t b = UNTOUCHED_SIZE;
        CHECK(fi_to_size(v, SIZE_MAX, &a) == (v < 0 ? FI_RANGE : FI_OK));
        CHECK(fd_from_signed(v, &b) == (v < 0 ? FD_RANGE : FD_OK));
        CHECK(a == b);
    }
    return 0;
}

int main(void)
{
    _Static_assert(SCHAR_MIN == -128 && SCHAR_MAX == 127,
                   "recorded narrow-destination experiment");
    unsigned long operations = 0;
    unsigned long extents = 0;

    if (compare_exhaustive_operations(&operations) ||
        check_primitive_semantics() || check_native_edges() ||
        compare_prior_extents(&extents) || compare_prior_conversion())
    {
        return 1;
    }
    printf(
        "FI_COMPARE_PASS operations=%lu extents=%lu edition=%ld native_int=%d narrow_char=%d\n",
        operations, extents, (long)__STDC_VERSION__, INT_MAX, SCHAR_MAX);
    return 0;
}
