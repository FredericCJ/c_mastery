#include "entry.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "FE_CHECK:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

/* Value stored in an output object before a call that must not publish. */
#define UNTOUCHED 91

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

/* Elements of the classroom allocator's single block. This is a property of
 * the test double, not an API limit of fe_staged(). */
enum
{
    POOL_CAPACITY = 64
};

/* Test allocator that hands out one block at a time and counts its use. */
struct fe_pool
{
    int32_t storage[POOL_CAPACITY];
    unsigned fail; /* nonzero: refuse every request */
    unsigned live; /* 1 while the block is lent out */
    unsigned takes; /* requests seen, granted or not */
    unsigned releases; /* blocks returned */
};

static int32_t *take(void *context, size_t count)
{
    struct fe_pool *pool = context;

    ++pool->takes;
    if (pool->fail || pool->live || count > POOL_CAPACITY)
    {
        return NULL;
    }
    pool->live = 1;
    return pool->storage;
}

static void release(void *context, int32_t *block)
{
    struct fe_pool *pool = context;

    CHECK(block == pool->storage && pool->live);
    pool->live = 0;
    ++pool->releases;
}

static struct fe_allocator allocator(struct fe_pool *pool)
{
    return (struct fe_allocator){pool, take, release};
}

/* Trace of [3, -1, 4]: four entering events (indices 0..3, index 3 being the
 * base case) then four returning events carrying the suffix sums
 * 0, 4, 3, 6. Both implementations must produce the same eight events. */
static void trace_of_three(void)
{
    const int32_t in[] = {3, -1, 4};
    int32_t a = UNTOUCHED;
    int32_t b = UNTOUCHED;
    struct fe_trace x;
    struct fe_trace y;

    CHECK(fe_recursive(in, 3, &a, &x) == FE_OK && a == 6);
    CHECK(fe_explicit_stack(in, 3, &b, &y) == FE_OK && b == 6);
    CHECK(x.count == 8 && y.count == 8 && x.maximum_live == 4 &&
          y.maximum_live == 4);

    const unsigned enters[] = {1, 1, 1, 1, 0, 0, 0, 0};
    const size_t indices[] = {0, 1, 2, 3, 3, 2, 1, 0};
    const int32_t values[] = {0, 0, 0, 0, 0, 4, 3, 6};
    for (size_t i = 0; i < ARRAY_LENGTH(enters); ++i)
    {
        CHECK(x.event[i].entering == enters[i] &&
              x.event[i].index == indices[i] && x.event[i].value == values[i]);
        CHECK(y.event[i].entering == enters[i] &&
              y.event[i].index == indices[i] && y.event[i].value == values[i]);
    }
}

static void trace_bounds(void)
{
    int32_t a = UNTOUCHED;
    int32_t full[FE_TRACE_MAX_ELEMENTS + 1] = {0};
    struct fe_trace x;
    struct fe_trace y;

    /* Empty input still enters and returns the base case: two events. */
    CHECK(fe_recursive(NULL, 0, &a, &x) == FE_OK && a == 0 && x.count == 2 &&
          x.maximum_live == 1);
    CHECK(fe_explicit_stack(NULL, 0, &a, &y) == FE_OK && a == 0 &&
          y.count == 2 && y.maximum_live == 1);

    /* Sixteen values: 17 activations, 34 events. Seventeen is refused. */
    CHECK(fe_recursive(full, 16, &a, &x) == FE_OK && x.count == 34 &&
          x.maximum_live == 17);
    CHECK(fe_explicit_stack(full, 16, &a, &y) == FE_OK && y.count == 34 &&
          y.maximum_live == 17);
    a = UNTOUCHED;
    CHECK(fe_recursive(full, 17, &a, &x) == FE_FULL && a == UNTOUCHED);
    CHECK(fe_explicit_stack(full, 17, &a, &y) == FE_FULL && a == UNTOUCHED);
}

static void trace(void)
{
    trace_of_three();
    trace_bounds();
    puts("FE_TRACE_PASS normal8 empty2 bounded34 "
         "explicit_activation_correspondence");
}

/* A failing request must leave the output untouched and must not leak the
 * staging block. [INT32_MAX, 1, -1] overflows at the second element even
 * though the final sum would fit. */
static void prefix_failures_preserve_output(void)
{
    const int32_t bad[] = {INT32_MAX, 1, -1};
    int32_t out[] = {91, 92, 93};
    struct fe_pool pool = {0};
    struct fe_allocator a = allocator(&pool);

    CHECK(fe_fixed(bad, 3, out, 3) == FE_RANGE && out[0] == 91 &&
          out[1] == 92 && out[2] == 93);
    CHECK(fe_two_pass(bad, 3, out, 3) == FE_RANGE && out[0] == 91 &&
          out[1] == 92 && out[2] == 93);
    CHECK(fe_staged(bad, 3, out, 3, a) == FE_RANGE && out[0] == 91 &&
          out[1] == 92 && out[2] == 93);
    CHECK(pool.live == 0 && pool.releases == 1);
}

static void prefix_successes_and_empties(void)
{
    const int32_t good[] = {3, -1, 4};
    int32_t out[] = {91, 92, 93};
    struct fe_pool pool = {0};
    struct fe_allocator a = allocator(&pool);

    CHECK(fe_fixed(good, 3, out, 3) == FE_OK && out[0] == 3 && out[1] == 2 &&
          out[2] == 6);
    CHECK(fe_fixed(NULL, 0, NULL, 0) == FE_OK);
    CHECK(fe_two_pass(NULL, 0, NULL, 0) == FE_OK);
    CHECK(fe_staged(NULL, 0, NULL, 0, (struct fe_allocator){0}) == FE_OK);

    /* An empty request must not even ask the allocator. */
    const unsigned before = pool.takes;
    CHECK(fe_staged(NULL, 0, NULL, 0, a) == FE_OK && pool.takes == before);

    /* Allocation refusal: output untouched, nothing left lent out. */
    pool.fail = 1;
    out[0] = UNTOUCHED;
    CHECK(fe_staged(good, 3, out, 3, a) == FE_ALLOC && out[0] == 91 &&
          pool.live == 0);
}

/* Overlap contracts differ: two_pass accepts exactly identical start
 * pointers, staged accepts any overlap. */
static void prefix_overlap(void)
{
    struct fe_pool pool = {0};
    struct fe_allocator a = allocator(&pool);
    int32_t inplace[] = {3, -1, 4};
    int32_t overlap[] = {3, -1, 4, 99};

    CHECK(fe_two_pass(inplace, 3, inplace, 3) == FE_OK && inplace[0] == 3 &&
          inplace[1] == 2 && inplace[2] == 6);
    CHECK(fe_staged(overlap, 3, overlap + 1, 3, a) == FE_OK &&
          overlap[0] == 3 && overlap[1] == 3 && overlap[2] == 2 &&
          overlap[3] == 6 && pool.live == 0);
}

/* Beyond the fixed primitive's 16 elements, and capacity rejections. */
static void prefix_extents(void)
{
    const int32_t good[] = {3, -1, 4};
    int32_t seventeen[17];
    int32_t result[17];
    int32_t small[2] = {81, 82};
    struct fe_pool pool = {0};
    struct fe_allocator a = allocator(&pool);

    for (size_t i = 0; i < ARRAY_LENGTH(seventeen); ++i)
    {
        seventeen[i] = 1;
        result[i] = UNTOUCHED;
    }
    CHECK(fe_fixed(seventeen, 17, result, 17) == FE_FULL && result[0] == 91);
    CHECK(fe_two_pass(seventeen, 17, result, 17) == FE_OK && result[16] == 17);
    CHECK(fe_staged(seventeen, 17, result, 17, a) == FE_OK && result[16] == 17);

    /* A byte count that would overflow size_t is refused before any request
     * reaches the allocator. */
    const unsigned before = pool.takes;
    CHECK(fe_staged(good, SIZE_MAX, small, SIZE_MAX, a) == FE_RANGE &&
          pool.takes == before);

    CHECK(fe_fixed(good, 3, small, 2) == FE_FULL && small[0] == 81 &&
          small[1] == 82);
    CHECK(fe_two_pass(good, 3, small, 2) == FE_FULL && small[0] == 81);
    CHECK(fe_staged(good, 3, small, 2, a) == FE_FULL && small[0] == 81);
}

static void maximum_cases(void)
{
    const int32_t good[] = {3, -1, 4};
    const int32_t negative[] = {-5, -2, -9};
    const int32_t minimum[] = {INT32_MIN};
    int32_t out[] = {UNTOUCHED};

    CHECK(fe_maximum(NULL, 0, &out[0]) == FE_EMPTY && out[0] == UNTOUCHED);
    CHECK(fe_maximum(good, 3, &out[0]) == FE_OK && out[0] == 4);
    CHECK(fe_maximum(negative, 3, &out[0]) == FE_OK && out[0] == -2);
    CHECK(fe_maximum(minimum, 1, &out[0]) == FE_OK && out[0] == INT32_MIN);
    CHECK(fe_maximum(NULL, 0, NULL) == FE_INVALID);
}

static void prefixes(void)
{
    prefix_failures_preserve_output();
    prefix_successes_and_empties();
    prefix_overlap();
    prefix_extents();
    maximum_cases();
    puts(
        "FE_PREFIX_PASS atomic_failure dynamic_rollback overlap17 maximum_empty");
}

static void encoding(void)
{
    unsigned char bytes[] = {91, 92, 93, 94};

    /* 0x1234ad is written most significant first; the fourth byte of the
     * four-byte buffer is outside the three-octet field and stays put. */
    CHECK(fe_encode24(UINT32_C(0x1234ad), bytes, 4) == FE_OK &&
          bytes[0] == 0x12 && bytes[1] == 0x34 && bytes[2] == 173 &&
          bytes[3] == 94);
    CHECK(fe_encode24(UINT32_C(0x1000000), bytes, 4) == FE_RANGE &&
          bytes[0] == 0x12);
    CHECK(fe_encode24(173, bytes, 2) == FE_FULL && bytes[0] == 0x12);
    CHECK(fe_encode24(0xffffff, bytes, 3) == FE_OK && bytes[0] == 255 &&
          bytes[1] == 255 && bytes[2] == 255);
    CHECK(fe_encode24(0, bytes, 3) == FE_OK && bytes[0] == 0 && bytes[1] == 0 &&
          bytes[2] == 0);
}

/* Per-bit oracle, deliberately not the bitwise expressions under test: counts
 * the bits selected by mask and how many of them are set in bits. */
static void count_mask_bits(unsigned bits, unsigned mask, unsigned *selected,
                            unsigned *present)
{
    *selected = 0;
    *present = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
        if ((mask / (1u << i)) % 2)
        {
            ++*selected;
            *present += (bits / (1u << i)) % 2;
        }
    }
}

static void mask_predicates(void)
{
    for (unsigned b = 0; b < 256; ++b)
    {
        for (unsigned m = 0; m < 256; ++m)
        {
            unsigned selected;
            unsigned present;

            count_mask_bits(b, m, &selected, &present);
            CHECK(fe_any((uint8_t)b, (uint8_t)m) == (present != 0));
            CHECK(fe_all((uint8_t)b, (uint8_t)m) == (present == selected));
            CHECK(fe_forbidden_clear((uint8_t)b, (uint8_t)m) == (present == 0));
        }
    }
}

/* Saturation is a policy, so it is not associative: clamping before or after
 * a compensating addition gives different results. */
static void saturation(void)
{
    CHECK(fe_saturating_add(fe_saturating_add(INT32_MAX, 1), -1) ==
          INT32_MAX - 1);
    CHECK(fe_saturating_add(INT32_MAX, fe_saturating_add(1, -1)) == INT32_MAX);
    CHECK(fe_saturating_add(INT32_MIN, -1) == INT32_MIN);
    CHECK(fe_saturating_add(-7, 4) == -3);
}

static void values(void)
{
    encoding();
    mask_predicates();
    saturation();

    /* Unsigned arithmetic is defined modulo 2^N, unlike the signed cases. */
    uint32_t wrapped = UINT32_MAX;
    wrapped += 1;
    CHECK(wrapped == 0);
    puts("FE_VALUE_PASS BE24 masks65536 saturation_nonassociative "
         "unsigned_modulo");
}

/* Returns 1 when every value lies in the int32_t range. */
static int all_fit_int32(const int64_t *sums, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        if (sums[i] < INT32_MIN || sums[i] > INT32_MAX)
        {
            return 0;
        }
    }
    return 1;
}

/* Prefix sums of in[0..3) must agree with the 64-bit formulas: the output is
 * all prefixes when every one fits, and untouched otherwise. */
static void check_prefix_oracle(const int32_t in[3], const int64_t sums[3])
{
    const int valid = all_fit_int32(sums, 3);
    const enum fe_status expected_status = valid ? FE_OK : FE_RANGE;
    struct fe_pool pool = {0};
    int32_t out[3] = {UNTOUCHED, UNTOUCHED, UNTOUCHED};
    int32_t other[3] = {UNTOUCHED, UNTOUCHED, UNTOUCHED};

    CHECK(fe_two_pass(in, 3, out, 3) == expected_status);
    CHECK(fe_staged(in, 3, other, 3, allocator(&pool)) == expected_status);
    CHECK(pool.live == 0);
    for (size_t i = 0; i < 3; ++i)
    {
        CHECK(out[i] == (valid ? (int32_t)sums[i] : 91) && other[i] == out[i]);
    }
}

/* The suffix sums have different intermediate values from the prefix sums,
 * so overflow can happen for one and not the other: the traced total uses
 * its own admissibility check. */
static void check_trace_oracle(const int32_t in[3], const int64_t sums[3])
{
    const int64_t suffix[3] = {(int64_t)in[2], (int64_t)in[1] + in[2], sums[2]};
    const int valid = all_fit_int32(suffix, 3);
    const enum fe_status expected_status = valid ? FE_OK : FE_RANGE;
    struct fe_trace x;
    struct fe_trace y;
    int32_t r = UNTOUCHED;
    int32_t s = UNTOUCHED;

    CHECK(fe_recursive(in, 3, &r, &x) == expected_status);
    CHECK(fe_explicit_stack(in, 3, &s, &y) == expected_status);
    CHECK(r == (valid ? (int32_t)sums[2] : 91) && s == r && x.count == y.count);
    for (size_t i = 0; i < x.count; ++i)
    {
        CHECK(x.event[i].index == y.event[i].index &&
              x.event[i].entering == y.event[i].entering &&
              x.event[i].value == y.event[i].value);
    }
}

/* Every triple drawn from seven extreme and small values (7^3 = 343). */
static void oracle(void)
{
    const int32_t choices[] = {INT32_MIN, -2, -1, 0, 1, 2, INT32_MAX};
    const size_t choice_count = ARRAY_LENGTH(choices);
    unsigned cases = 0;

    for (size_t a = 0; a < choice_count; ++a)
    {
        for (size_t b = 0; b < choice_count; ++b)
        {
            for (size_t c = 0; c < choice_count; ++c)
            {
                const int32_t in[3] = {choices[a], choices[b], choices[c]};
                const int64_t sums[3] = {(int64_t)in[0], (int64_t)in[0] + in[1],
                                         (int64_t)in[0] + in[1] + in[2]};

                check_prefix_oracle(in, sums);
                check_trace_oracle(in, sums);
                ++cases;
            }
        }
    }
    printf("FE_ORACLE_PASS direct_integer_formula_cases=%u\n", cases);
}

int main(void)
{
    trace();
    prefixes();
    values();
    oracle();
    puts("FE_PASS entry_algorithms_values");
    return 0;
}
