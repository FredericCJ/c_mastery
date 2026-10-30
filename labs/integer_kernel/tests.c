#include "integer.h"
#include <limits.h>
#include <stdio.h>
#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            fprintf(stderr, "FI_FAIL %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                      \
        }                                                                  \
    } while (0)

/*
 * Failure atomicity is checked with sentinels: an output the function must not
 * touch is preset to a distinctive value (917, 918, 919) and compared after the
 * call. The expressions inside CHECK are matched as text by the mutation
 * campaign, so their spelling is part of the test contract.
 */
static const struct fi_range byte = {-128, 127};
static const struct fi_range native = {INT_MIN, INT_MAX};

/* True when INT_MIN has no positive counterpart (two's complement), so
 * INT_MIN * -1 and INT_MIN / -1 are not representable in int. */
#define ASYMMETRIC_INT (INT_MIN < -INT_MAX)

/* Checked add, subtract and multiply, then saturation. */
static int binary_and_saturate(void)
{
    int out = 917;
    unsigned clipped = 9;
    CHECK(fi_binary(byte, FI_ADD, 120, 10, &out) == FI_RANGE && out == 917);
    CHECK(fi_binary(native, FI_ADD, 120, 10, &out) == FI_OK && out == 130);
    CHECK(((250u + 10u) & 255u) == 4u);
    out = 917;
    CHECK(fi_binary(byte, FI_SUBTRACT, -128, 1, &out) == FI_RANGE &&
          out == 917);
    CHECK(fi_binary(byte, FI_MULTIPLY, -12, 10, &out) == FI_OK && out == -120);
    CHECK(fi_saturate(byte, FI_ADD, 120, 10, &out, &clipped) == FI_OK &&
          out == 127 && clipped == 1);
    CHECK(fi_saturate(byte, FI_SUBTRACT, -128, 1, &out, &clipped) == FI_OK &&
          out == -128 && clipped == 1);
    CHECK(fi_saturate(byte, FI_MULTIPLY, -12, 10, &out, &clipped) == FI_OK &&
          out == -120 && clipped == 0);
    out = 917;
    CHECK(fi_binary(native, FI_ADD, INT_MAX, 1, &out) == FI_RANGE &&
          out == 917);
    CHECK(fi_binary(native, FI_SUBTRACT, INT_MIN, 1, &out) == FI_RANGE &&
          out == 917);
    CHECK(fi_binary(native, FI_MULTIPLY, INT_MIN, -1, &out) ==
          (ASYMMETRIC_INT ? FI_RANGE : FI_OK));
    CHECK(out == (ASYMMETRIC_INT ? 917 : INT_MAX));
    CHECK(fi_binary(native, FI_MULTIPLY, INT_MIN, 0, &out) == FI_OK &&
          out == 0);
    CHECK(fi_binary(native, FI_MULTIPLY, INT_MIN, 1, &out) == FI_OK &&
          out == INT_MIN);
    return 0;
}

/* Truncating, floor and Euclidean division, including unrepresentable
 * quotients and a zero divisor. */
static int division(void)
{
    int q = 918;
    int r = 919;
    const int input[][2] = {{-7, 3}, {7, -3}, {-7, -3}, {7, 3}};
    const int quotients[] = {-2, -2, 2, 2};
    const int remainders[] = {-1, 1, -1, 1};
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(fi_divide(byte, FI_TRUNCATE, input[i][0], input[i][1], &q, &r) ==
              FI_OK);
        CHECK(q == quotients[i] && r == remainders[i]);
    }
    q = 918;
    r = 919;
    CHECK(fi_divide(byte, FI_TRUNCATE, -128, -1, &q, &r) == FI_RANGE &&
          q == 918 && r == 919);
    CHECK(fi_divide(native, FI_TRUNCATE, INT_MIN, -1, &q, &r) ==
          (ASYMMETRIC_INT ? FI_RANGE : FI_OK));
    CHECK(q == (ASYMMETRIC_INT ? 918 : INT_MAX) &&
          r == (ASYMMETRIC_INT ? 919 : 0));
    q = 918;
    r = 919;
    CHECK(fi_divide(native, FI_TRUNCATE, 1, 0, &q, &r) == FI_RANGE &&
          q == 918 && r == 919);
    CHECK(fi_divide((struct fi_range){-127, 127}, FI_TRUNCATE, -127, -1, &q,
                    &r) == FI_OK &&
          q == 127 && r == 0);
    CHECK(fi_divide(byte, FI_FLOOR, -7, 3, &q, &r) == FI_OK && q == -3 &&
          r == 2);
    CHECK(fi_divide(byte, FI_FLOOR, 7, -3, &q, &r) == FI_OK && q == -3 &&
          r == -2);
    CHECK(fi_divide(byte, FI_EUCLIDEAN, -7, -3, &q, &r) == FI_OK && q == 3 &&
          r == 2);
    CHECK(fi_divide(native, FI_EUCLIDEAN, -1, INT_MIN, &q, &r) == FI_OK &&
          q == 1 && r == -(INT_MIN + 1));
    return 0;
}

/* Non-negative modulus, then every invalid-argument path leaves the output
 * untouched. */
static int modulus_and_invalid_arguments(void)
{
    int out = 917;
    CHECK(fi_mod_positive(-7, 3, &out) == FI_OK && out == 2);
    CHECK(fi_mod_positive(INT_MIN, INT_MAX, &out) == FI_OK &&
          out == (ASYMMETRIC_INT ? INT_MAX - 1 : 0));
    out = 917;
    CHECK(fi_mod_positive(1, 0, &out) == FI_INVALID && out == 917);
    CHECK(fi_mod_positive(1, -3, &out) == FI_INVALID && out == 917);
    CHECK(fi_divide(byte, FI_TRUNCATE, 7, 3, &out, &out) == FI_INVALID &&
          out == 917);
    CHECK(fi_binary((struct fi_range){1, 2}, FI_ADD, 1, 1, &out) ==
              FI_INVALID &&
          out == 917);
    CHECK(fi_binary(byte, FI_ADD, 128, 1, &out) == FI_RANGE && out == 917);
    CHECK(fi_binary(byte, (enum fi_operation)99, 1, 1, &out) == FI_INVALID &&
          out == 917);
    return 0;
}

static int arithmetic(void)
{
    if (binary_and_saturate() || division() || modulus_and_invalid_arguments())
    {
        return 1;
    }
    puts(
        "FI_ARITHMETIC_PASS reject_saturate sign_partitions division_conventions symmetric_range");
    return 0;
}

/* Signed-to-size conversion and checked extents. */
static int sizes_and_extents(void)
{
    size_t out = 917;
    CHECK(fi_to_size(-1, SIZE_MAX, &out) == FI_RANGE && out == 917);
    CHECK(fi_to_size(0, SIZE_MAX, &out) == FI_OK && out == 0);
    CHECK(fi_to_size(INTMAX_MAX, SIZE_MAX, &out) ==
          ((uintmax_t)INTMAX_MAX > SIZE_MAX ? FI_RANGE : FI_OK));
    out = 917;
    CHECK(fi_to_size(65536, 65535, &out) == FI_RANGE && out == 917);
    CHECK(fi_to_size(65535, 65535, &out) == FI_OK && out == 65535);
    out = 917;
    CHECK(fi_layout(12, -1, 9, 180, &out) == FI_RANGE && out == 917);
    CHECK(fi_layout(12, 19, 9, 180, &out) == FI_RANGE && out == 917);
    CHECK(fi_layout(12, 18, 9, 180, &out) == FI_OK && out == 174);
    out = 917;
    CHECK(fi_extent(SIZE_MAX, 1, 1, SIZE_MAX, &out) == FI_RANGE && out == 917);
    CHECK(fi_extent(SIZE_MAX, SIZE_MAX, 0, SIZE_MAX, &out) == FI_OK &&
          out == SIZE_MAX);
    out = 917;
    CHECK(fi_extent(1, 0, 0, 0, &out) == FI_RANGE && out == 917);
    return 0;
}

/* Logical offsets: a window position is a number, not a pointer difference.
 * The base is chosen above PTRDIFF_MAX to show that no pointer is involved. */
static int logical_windows(void)
{
    size_t out = 917;
    int local[4] = {11, 22, 33, 44};
    uintmax_t base = (uintmax_t)PTRDIFF_MAX + 1;
    CHECK(fi_window(base, base + 2, 4, &out) == FI_OK && out == 2 &&
          local[out] == 33);
    ptrdiff_t delta = &local[1] - &local[3];
    CHECK(delta == -2);
    out = 917;
    CHECK(fi_to_size((intmax_t)delta, SIZE_MAX, &out) == FI_RANGE &&
          out == 917);
    CHECK(fi_window(base, base - 1, 4, &out) == FI_RANGE && out == 917);
    CHECK(fi_window(base, base + 5, 4, &out) == FI_RANGE && out == 917);
    return 0;
}

/* The 32-bit wire word as four logical octets, in cells and in bytes. */
static int octet_cells(void)
{
    unsigned int cells[4];
    uint_least32_t decoded = 917;
    unsigned char bytes[4] = {9, 9, 9, 9};
    CHECK(fi_encode_cells(UINT32_C(0x12345678), cells) == FI_OK);
    CHECK(cells[0] == 18 && cells[1] == 52 && cells[2] == 86 &&
          cells[3] == 120);
    CHECK(fi_decode_cells(cells, &decoded) == FI_OK &&
          decoded == UINT32_C(0x12345678));
    cells[2] = 256;
    decoded = 917;
    CHECK(fi_decode_cells(cells, &decoded) == FI_RANGE && decoded == 917);
    CHECK(fi_encode_octets(UINT32_C(0x12345678), bytes) ==
          (CHAR_BIT == 8 ? FI_OK : FI_UNSUPPORTED));
    CHECK(CHAR_BIT == 8 ? bytes[0] == 18 && bytes[3] == 120
                        : bytes[0] == 9 && bytes[3] == 9);
    return 0;
}

static int boundaries(void)
{
    if (sizes_and_extents() || logical_windows() || octet_cells())
    {
        return 1;
    }
    puts(
        "FI_BOUNDARY_PASS signed_before_size narrow_count checked_layout logical_offsets octet_cells");
    return 0;
}

int main(void)
{
    if (arithmetic() || boundaries())
    {
        return 1;
    }
    puts("FI_PASS integer_kernel");
    return 0;
}
