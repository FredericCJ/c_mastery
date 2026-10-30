#include "semantics.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "FS_CHECK:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

/* Value stored in an output object before a call that must not publish. */
#define UNTOUCHED 91

/* Bounds of the exhaustive metadata sweep: every length in 0..MAX_LENGTH
 * against every index in MIN_INDEX..MAX_INDEX (9 * 13 = 117 cases). */
enum
{
    MAX_LENGTH = 8,
    MIN_INDEX = -2,
    MAX_INDEX = 10
};

/* Independent statement of the contract for the sweep. */
static enum fs_index expected_class(int index, int length)
{
    if (index < 0 || index > length)
    {
        return FS_OUTSIDE;
    }
    if (index == length)
    {
        return FS_ONE_PAST;
    }
    return FS_ELEMENT;
}

static void index_classes(void)
{
    const intmax_t indices[5] = {-1, 0, 2, 3, 4};

    const enum fs_index expected[5] = {FS_OUTSIDE, FS_ELEMENT, FS_ELEMENT,
                                       FS_ONE_PAST, FS_OUTSIDE};

    for (unsigned i = 0; i < 5; i++)
    {
        CHECK(fs_index(indices[i], 3) == expected[i]);
    }
    CHECK(fs_index(INTMAX_MIN, 3) == FS_OUTSIDE);
    CHECK(fs_index(0, 0) == FS_ONE_PAST);
}

/* Returns the number of (length, index) pairs checked. */
static unsigned index_sweep(void)
{
    unsigned cases = 0;

    for (int length = 0; length <= MAX_LENGTH; length++)
    {
        for (int index = MIN_INDEX; index <= MAX_INDEX; index++)
        {
            const enum fs_index expected = expected_class(index, length);
            CHECK(fs_index(index, (size_t)length) == expected);
            ++cases;
        }
    }
    return cases;
}

static void sums_and_rows(void)
{
    const int row[3] = {1, 2, 3};
    int out = UNTOUCHED;

    CHECK(fs_sum(row, 3, &out) == FS_OK && out == 6);
    CHECK(fs_row(&row, &out) == FS_OK && out == 6);
    CHECK(fs_sum(NULL, 0, &out) == FS_OK && out == 0);

    const int overflowing[3] = {INT_MAX, 1, -1};
    out = UNTOUCHED;
    CHECK(fs_sum(overflowing, 3, &out) == FS_RANGE && out == UNTOUCHED);

    /* Only legal pointer formation is executed; no dereference of one-past. */
    const int *one_past = row + 3;
    CHECK(one_past - row == 3 && one_past[-1] == 3);
}

static void matrices(void)
{
    const int matrix[2][3] = {{1, 2, 3}, {10, 20, 30}};
    int out = UNTOUCHED;

    CHECK(fs_matrix(matrix, 2, &out) == FS_OK && out == 66);
    CHECK(fs_matrix(NULL, 0, &out) == FS_OK && out == 0);

    const int overflowing[2][3] = {{INT_MAX, 0, 0}, {1, -1, 0}};
    out = UNTOUCHED;
    CHECK(fs_matrix(overflowing, 2, &out) == FS_RANGE && out == UNTOUCHED);

    /* Adding one to a row pointer moves by a whole row, not by one int. */
    CHECK(sizeof *matrix == 3 * sizeof(int) && matrix + 1 == &matrix[1]);
}

static void arrays(void)
{
    index_classes();
    sums_and_rows();
    matrices();
    const unsigned cases = index_sweep();

    printf("FS_ARRAY_PASS metadata_cases=%u typed_rows actual_stride "
           "failure_atomic\n",
           cases);
}

static void paths(void)
{
    int out = UNTOUCHED;

    CHECK(fs_branch(1, &out) == FS_OK && out == 7);
    CHECK(fs_branch(0, &out) == FS_OK && out == 11);
    out = UNTOUCHED;
    CHECK(fs_branch(2, &out) == FS_INVALID && out == UNTOUCHED);
    CHECK(fs_branch(0, NULL) == FS_INVALID);
    puts("FS_PATH_PASS both_branches invalid_preserved "
         "no_uninitialized_execution");
}

static void sequence(void)
{
    struct fs_calls result;

    CHECK(fs_calls(&result) == FS_OK && result.sum == 6 && result.x == 2 &&
          result.y == 4);
    CHECK(result.order == 12 || result.order == 21);

    int a = 91;
    int b = 92;
    CHECK(fs_comma(&a, &b) == FS_OK && a == 2 && b == 3);
    CHECK(fs_comma(&a, &a) == FS_INVALID && a == 2);

    int zero = 0;
    /* A zero left operand prevents evaluating the right operand (1 / zero). */
    CHECK((zero && (1 / zero)) == 0);
    printf("FS_SEQUENCE_PASS observed_call_order=%u permitted=12,21 comma=2,3 "
           "short_circuit=0\n",
           result.order);
}

int main(void)
{
    arrays();
    paths();
    sequence();
    puts("FS_PASS defined_repairs_and_transfer");
    return 0;
}
