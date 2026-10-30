/*
 * Native observations of the host's integer types, promotions and conversions.
 * Every line printed here is an observation of one compiler and one data model,
 * not a portable golden result; the _Static_assert below records the model the
 * checks assume.
 */
#include "integer.h"
#include <limits.h>
#include <stdio.h>
#define CHECK(x)                                                      \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "FI_OBSERVE_FAIL %d %s\n", __LINE__, #x); \
            return 1;                                                 \
        }                                                             \
    } while (0)
#define ROW(type, lo, hi)                                                 \
    printf("TYPE %s storage=%zu low=%jd high=%ju fits100000=%d\n", #type, \
           sizeof(type), (intmax_t)(lo), (uintmax_t)(hi),                 \
           (uintmax_t)(hi) >= UINTMAX_C(100000))
#define EXPR(x)                                           \
    printf("EXPR %s type=%s value=%jd\n", #x,             \
           _Generic((x),                                  \
               int: "int",                                \
               unsigned int: "unsigned int",              \
               long: "long",                              \
               unsigned long: "unsigned long",            \
               long long: "long long",                    \
               unsigned long long: "unsigned long long"), \
           (intmax_t)(x))

/* Number of boundary cases the size test could execute: SIZE_MAX itself and
 * SIZE_MAX + 1. Each needs a source type wide enough to hold it. */
#define SIZE_BOUNDARY_CASES 2u

/* Sentinel proving a function left its output untouched on failure. */
#define UNTOUCHED 917

static void print_type_table(void)
{
    printf("MODEL CHAR_BIT=%d SIZE_MAX=%ju PTRDIFF_MAX=%jd INTMAX_MAX=%jd\n",
           CHAR_BIT, (uintmax_t)SIZE_MAX, (intmax_t)PTRDIFF_MAX, INTMAX_MAX);
    ROW(char, CHAR_MIN, CHAR_MAX);
    ROW(signed char, SCHAR_MIN, SCHAR_MAX);
    ROW(unsigned char, 0, UCHAR_MAX);
    ROW(short, SHRT_MIN, SHRT_MAX);
    ROW(unsigned short, 0, USHRT_MAX);
    ROW(int, INT_MIN, INT_MAX);
    ROW(unsigned int, 0, UINT_MAX);
    ROW(long, LONG_MIN, LONG_MAX);
    ROW(unsigned long, 0, ULONG_MAX);
    ROW(long long, LLONG_MIN, LLONG_MAX);
    ROW(unsigned long long, 0, ULLONG_MAX);
}

/* Integer promotion: an unsigned char operand becomes int, so the results
 * below are negative or larger than the operand's own range. */
static int check_promotions(void)
{
    unsigned char x = 255;
    EXPR(x + 1);
    EXPR(~x);
    EXPR(x << 8);
    EXPR(100000);
    EXPR(65535);
    EXPR(0xffff);
    CHECK(x + 1 == 256 && ~x == -256 && (x << 8) == 65280);
    return 0;
}

/* Usual arithmetic conversions: a negative signed value compared with an
 * unsigned one of the same rank is converted, not the other way round. */
static int check_signed_unsigned_comparisons(void)
{
    int negative = -1;
    unsigned int one = 1u;
    size_t capacity = 5;
    CHECK((unsigned int)negative == UINT_MAX &&
          !((unsigned int)negative < one));
    CHECK((size_t)negative == SIZE_MAX && !((size_t)negative < capacity));
    long long wide = -1;
    CHECK(LLONG_MAX > UINT_MAX && wide < one);
    return 0;
}

/* A bit-field narrower than int promotes to int; a full-width one keeps its
 * unsigned type. Neither has an address. */
static int check_bitfield_promotion(void)
{
    struct
    {
        unsigned int small : 8;
        unsigned int full : 32;
    } fields = {255, UINT_MAX};

    CHECK(_Generic(+fields.small, int: 1, default: 0));
    CHECK(_Generic(+fields.full, unsigned int: 1, default: 0));
    CHECK(+fields.small == 255 && +fields.full == UINT_MAX);
    return 0;
}

/* Run the SIZE_MAX boundary cases only where intmax_t can represent the
 * source value; otherwise report them as unavailable instead of inventing a
 * result. On the recorded model INTMAX_MAX < SIZE_MAX, so both are skipped. */
static int check_size_boundaries(void)
{
    size_t out = UNTOUCHED;
    unsigned executed = 0;

    if ((uintmax_t)INTMAX_MAX >= (uintmax_t)SIZE_MAX)
    {
        CHECK(fi_to_size((intmax_t)SIZE_MAX, SIZE_MAX, &out) == FI_OK &&
              out == SIZE_MAX);
        ++executed;
    }
    if ((uintmax_t)INTMAX_MAX > (uintmax_t)SIZE_MAX)
    {
        out = UNTOUCHED;
        CHECK(fi_to_size((intmax_t)((uintmax_t)SIZE_MAX + 1), SIZE_MAX, &out) ==
                  FI_RANGE &&
              out == UNTOUCHED);
        ++executed;
    }
    printf("FI_SIZE_BOUNDARIES executed=%u unavailable_source_values=%u\n",
           executed, SIZE_BOUNDARY_CASES - executed);
    return 0;
}

int main(void)
{
    /* These executions require the recorded two's-complement, int32 host
     * model. The int16 signed-left-shift case exists only in the mathematical
     * dossier. */
    _Static_assert(UCHAR_MAX == 255 && INT_MAX == 2147483647 &&
                       INT_MIN == (-2147483647 - 1),
                   "native observation assumptions");

    print_type_table();
    if (check_promotions() || check_signed_unsigned_comparisons() ||
        check_bitfield_promotion() || check_size_boundaries())
    {
        return 1;
    }
    puts(
        "FI_OBSERVE_PASS actual_limits promotions bitfields wider_signed local_pointer");
    return 0;
}
