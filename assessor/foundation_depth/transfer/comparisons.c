/* Comparison harness: the checked extent against compiler overflow builtins
 * (and C23 <stdckdint.h> when FD_C23 is defined), and the restricted transform
 * against an unrestricted twin. Prints FD_CPU_SAMPLE lines for the CPU
 * comparison and "FD_COMPARE_OK:<cases> ..." on success. Timing samples are
 * observations, never pass/fail criteria. */
#include "depth.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef FD_C23
#include <stdckdint.h>
#endif

#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "FD_COMPARE_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                                  \
        }                                                              \
    } while (0)

#define UNTOUCHED_SIZE ((size_t)917)
/* Each exhaustive extent argument runs 0..GRID_MAX-1 (16^4 = 65536 cases). */
#define GRID_MAX 16u
#define TIMING_SIZES 4u
#define TIMING_SAMPLES 5u
#define TIMING_REPETITIONS 128u
#define TIMING_BUFFER_BYTES 65536u

static enum fd_status builtin_extent(size_t offset, size_t count, size_t width,
                                     size_t limit, size_t *out)
{
    size_t product;
    size_t end;
    if (!out)
    {
        return FD_INVALID;
    }
    if (__builtin_mul_overflow(count, width, &product) ||
        __builtin_add_overflow(offset, product, &end) || end > limit)
    {
        return FD_RANGE;
    }
    *out = end;
    return FD_OK;
}

#ifdef FD_C23
static enum fd_status checked_extent(size_t offset, size_t count, size_t width,
                                     size_t limit, size_t *out)
{
    size_t product;
    size_t end;
    if (!out)
    {
        return FD_INVALID;
    }
    if (ckd_mul(&product, count, width) || ckd_add(&end, offset, product) ||
        end > limit)
    {
        return FD_RANGE;
    }
    *out = end;
    return FD_OK;
}
#endif

/* Defined in unrestricted.c, a separate translation unit. */
void fd_plain_transform(unsigned char *, const unsigned char *, size_t,
                        unsigned char);

/* Every small argument combination: all implementations agree with exact
 * arithmetic and with each other. Adds to *cases. */
static int compare_extent_grid(unsigned long *cases)
{
    for (size_t offset = 0; offset < GRID_MAX; ++offset)
    {
        for (size_t count = 0; count < GRID_MAX; ++count)
        {
            for (size_t width = 0; width < GRID_MAX; ++width)
            {
                for (size_t limit = 0; limit < GRID_MAX; ++limit)
                {
                    size_t library_out = UNTOUCHED_SIZE;
                    size_t builtin_out = UNTOUCHED_SIZE;
                    const size_t exact = offset + count * width;
                    const enum fd_status want =
                        exact <= limit ? FD_OK : FD_RANGE;
                    CHECK(fd_extent(offset, count, width, limit,
                                    &library_out) == want);
                    CHECK(builtin_extent(offset, count, width, limit,
                                         &builtin_out) == want &&
                          library_out == builtin_out);
#ifdef FD_C23
                    size_t checked_out = UNTOUCHED_SIZE;
                    CHECK(checked_extent(offset, count, width, limit,
                                         &checked_out) == want &&
                          library_out == checked_out);
#endif
                    CHECK(library_out ==
                          (want == FD_OK ? exact : UNTOUCHED_SIZE));
                    ++*cases;
                }
            }
        }
    }
    return 0;
}

/* Arguments at the edge of size_t, where the exact sum or product wraps. The
 * builtin (and C23) versions must agree with the expected outcomes. */
static int compare_extent_edges(unsigned long *cases)
{
    const size_t edge[][4] = {
        {SIZE_MAX, 1, 1, SIZE_MAX},        {0, SIZE_MAX, 2, SIZE_MAX},
        {SIZE_MAX, SIZE_MAX, 0, SIZE_MAX}, {SIZE_MAX, 0, 0, SIZE_MAX - 1},
        {SIZE_MAX - 1, 1, 1, SIZE_MAX},    {0, 0, SIZE_MAX, 0}};

    const enum fd_status want[] = {FD_RANGE, FD_RANGE, FD_OK,
                                   FD_RANGE, FD_OK,    FD_OK};

    const size_t result[] = {UNTOUCHED_SIZE, UNTOUCHED_SIZE, SIZE_MAX,
                             UNTOUCHED_SIZE, SIZE_MAX,       0};
    for (unsigned i = 0; i < sizeof edge / sizeof edge[0]; ++i)
    {
        size_t out = UNTOUCHED_SIZE;
        CHECK(builtin_extent(edge[i][0], edge[i][1], edge[i][2], edge[i][3],
                             &out) == want[i] &&
              out == result[i]);
#ifdef FD_C23
        out = UNTOUCHED_SIZE;
        CHECK(checked_extent(edge[i][0], edge[i][1], edge[i][2], edge[i][3],
                             &out) == want[i] &&
              out == result[i]);
#endif
        ++*cases;
    }
    size_t written = UNTOUCHED_SIZE;
    CHECK(__builtin_add_overflow(SIZE_MAX, (size_t)1, &written) &&
          written == 0);
#ifdef FD_C23
    written = UNTOUCHED_SIZE;
    CHECK(ckd_add(&written, SIZE_MAX, (size_t)1) && written == 0);
#endif
    return 0;
}

static int clock_valid(clock_t ticks)
{
    return ticks != (clock_t)-1;
}

/* Runs TIMING_REPETITIONS transforms and returns a checksum of one output
 * byte per repetition, so the optimizer cannot discard the work. */
static unsigned long run_restricted(unsigned char *buffer,
                                    const unsigned char *source, size_t length)
{
    unsigned long checksum = 0;
    for (unsigned rep = 0; rep < TIMING_REPETITIONS; ++rep)
    {
        fd_transform(buffer, source, length, (unsigned char)rep);
        checksum += buffer[rep % length];
    }
    return checksum;
}

static unsigned long run_plain(unsigned char *buffer,
                               const unsigned char *source, size_t length)
{
    unsigned long checksum = 0;
    for (unsigned rep = 0; rep < TIMING_REPETITIONS; ++rep)
    {
        fd_plain_transform(buffer, source, length, (unsigned char)rep);
        checksum += buffer[rep % length];
    }
    return checksum;
}

/* Times the restricted and plain transforms back to back, alternating which
 * one goes first on odd samples to spread warm-up effects. */
static int compare_transforms(unsigned long *checksum)
{
    static unsigned char source[TIMING_BUFFER_BYTES];
    static unsigned char restricted[TIMING_BUFFER_BYTES];
    static unsigned char unrestricted[TIMING_BUFFER_BYTES];
    for (size_t i = 0; i < sizeof source; ++i)
    {
        source[i] = (unsigned char)i;
    }
    const size_t sizes[TIMING_SIZES] = {4, 64, 4096, TIMING_BUFFER_BYTES};
    for (unsigned size_index = 0; size_index < TIMING_SIZES; ++size_index)
    {
        for (unsigned sample = 0; sample < TIMING_SAMPLES; ++sample)
        {
            const size_t length = sizes[size_index];
            const int plain_first = sample % 2;
            /* Warm both paths before timing. */
            fd_transform(restricted, source, length, 0);
            fd_plain_transform(unrestricted, source, length, 0);

            const clock_t begin = clock();
            CHECK(clock_valid(begin));
            *checksum += plain_first
                             ? run_plain(unrestricted, source, length)
                             : run_restricted(restricted, source, length);
            const clock_t middle = clock();
            CHECK(clock_valid(middle));
            *checksum += plain_first
                             ? run_restricted(restricted, source, length)
                             : run_plain(unrestricted, source, length);
            const clock_t end = clock();
            CHECK(clock_valid(end));

            CHECK(!memcmp(restricted, unrestricted, length));
            const clock_t first_ticks = middle - begin;
            const clock_t second_ticks = end - middle;
            printf(
                "FD_CPU_SAMPLE size=%zu repetitions=%u sample=%u restricted_ticks=%ld plain_ticks=%ld clocks_per_second=%ld\n",
                length, TIMING_REPETITIONS, sample,
                (long)(plain_first ? second_ticks : first_ticks),
                (long)(plain_first ? first_ticks : second_ticks),
                (long)CLOCKS_PER_SEC);
        }
    }
    return 0;
}

int main(void)
{
    unsigned long cases = 0;
    unsigned long checksum = 0;
    if (compare_extent_grid(&cases) != 0 || compare_extent_edges(&cases) != 0 ||
        compare_transforms(&checksum) != 0)
    {
        return 1;
    }
    printf("FD_COMPARE_OK:%lu checksum=%lu edition=%ld\n", cases, checksum,
           (long)__STDC_VERSION__);
    return 0;
}
