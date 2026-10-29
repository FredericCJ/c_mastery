#include "integer.h"
#include "depth.h" /* Actual unchanged prerequisite extent/conversion provider. */
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

static enum fi_status builtin(struct fi_range r, enum fi_operation op, int a,
                              int b, int *out)
{
    int temporary;
    int overflow = op == FI_ADD        ? ADD(&temporary, a, b)
                   : op == FI_SUBTRACT ? SUB(&temporary, a, b)
                                       : MUL(&temporary, a, b);
#ifdef FI_BUILTIN_EARLY
    *out = temporary;
#endif
    if (overflow || temporary < r.low || temporary > r.high)
        return FI_RANGE;
    *out = temporary;
    return FI_OK;
}

static enum fi_status narrow(enum fi_operation op, int a, int b,
                             signed char *out)
{
    signed char temporary;
    int overflow = op == FI_ADD        ? ADD(&temporary, a, b)
                   : op == FI_SUBTRACT ? SUB(&temporary, a, b)
                                       : MUL(&temporary, a, b);
    if (overflow)
        return FI_RANGE;
    *out = temporary;
    return FI_OK;
}

int main(void)
{
    _Static_assert(SCHAR_MIN == -128 && SCHAR_MAX == 127,
                   "recorded narrow-destination experiment");
    unsigned long operations = 0, extents = 0;
    for (int a = -128; a <= 127; ++a)
        for (int b = -128; b <= 127; ++b)
            for (unsigned op = 0; op < 3; ++op)
            {
                int exact = op == FI_ADD        ? a + b
                            : op == FI_SUBTRACT ? a - b
                                                : a * b;
                enum fi_status expected =
                    exact >= -128 && exact <= 127 ? FI_OK : FI_RANGE;
                int manual = 917, extension = 917;
                signed char small = 91;
                CHECK(fi_binary((struct fi_range){-128, 127},
                                (enum fi_operation)op, a, b,
                                &manual) == expected);
                CHECK(builtin((struct fi_range){-128, 127},
                              (enum fi_operation)op, a, b,
                              &extension) == expected);
                CHECK(manual == extension &&
                      extension == (expected == FI_OK ? exact : 917));
                CHECK(narrow((enum fi_operation)op, a, b, &small) == expected &&
                      small == (expected == FI_OK ? exact : 91));
                ++operations;
            }
    /* Primitive overflow stores a result; public failure-atomic wrappers must stage. */
    unsigned int wrapped = 91;
    CHECK(ADD(&wrapped, UINT_MAX, 1u) && wrapped == 0);
    signed char negative = 91;
    CHECK(!MUL(&negative, -1, 1u) && negative == -1);
    CHECK(
        (-1 * 1u) ==
        UINT_MAX); /* Ordinary conversions differ from mathematical builtin semantics. */
    int edge = 917;
    CHECK(builtin((struct fi_range){INT_MIN, INT_MAX}, FI_ADD, INT_MAX, 1,
                  &edge) == FI_RANGE &&
          edge == 917);
    CHECK(builtin((struct fi_range){INT_MIN, INT_MAX}, FI_SUBTRACT, INT_MIN, 1,
                  &edge) == FI_RANGE &&
          edge == 917);
    CHECK(builtin((struct fi_range){INT_MIN, INT_MAX}, FI_MULTIPLY, INT_MIN, -1,
                  &edge) == FI_RANGE &&
          edge == 917);
    /* Composition uses actual existing fd_extent/fd_from_signed, not renamed local substitutes. */
    for (size_t p = 0; p < 9; ++p)
        for (size_t n = 0; n < 9; ++n)
            for (size_t width = 0; width < 9; ++width)
                for (size_t limit = 0; limit < 9; ++limit)
                {
                    size_t a = 917, b = 917, exact = p + n * width;
                    CHECK(fi_extent(p, n, width, limit, &a) ==
                          (exact <= limit ? FI_OK : FI_RANGE));
                    CHECK(fd_extent(p, n, width, limit, &b) ==
                          (exact <= limit ? FD_OK : FD_RANGE));
                    CHECK(a == b && a == (exact <= limit ? exact : 917));
                    ++extents;
                }
    for (intmax_t v = -9; v <= 9; ++v)
    {
        size_t a = 917, b = 917;
        CHECK(fi_to_size(v, SIZE_MAX, &a) == (v < 0 ? FI_RANGE : FI_OK));
        CHECK(fd_from_signed(v, &b) == (v < 0 ? FD_RANGE : FD_OK));
        CHECK(a == b);
    }
    printf(
        "FI_COMPARE_PASS operations=%lu extents=%lu edition=%ld native_int=%d narrow_char=%d\n",
        operations, extents, (long)__STDC_VERSION__, INT_MAX, SCHAR_MAX);
    return 0;
}
