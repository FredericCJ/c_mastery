/* Reference implementation of the semantics.h contract. Assessor-only.
 *
 * FS_MUTANT selects a wrong-but-defined variant for the mutation campaign
 * (0 is the correct reference); every mutant stays free of undefined
 * behaviour so it can be executed and rejected by the public tests. */
#include "semantics.h"
#include <limits.h>
#ifndef FS_MUTANT
#define FS_MUTANT 0
#endif

/* Ints per row in the fs_row() and fs_matrix() interfaces. */
enum
{
    ROW_LENGTH = 3
};

/* Decimal encoding of call order in struct fs_calls.order: each call body
 * appends its own digit, so first-then-second reads 12. */
enum
{
    ORDER_BASE = 10,
    ORDER_DIGIT_FIRST = 1,
    ORDER_DIGIT_SECOND = 2
};

enum fs_index fs_index(intmax_t index, size_t length)
{
    /* Test the sign in signed arithmetic first; only a non-negative index may
     * be converted to the unsigned type, and then without loss. */
    if (index < 0 || (uintmax_t)index > (uintmax_t)length)
    {
        return FS_OUTSIDE;
    }
    if ((uintmax_t)index == (uintmax_t)length)
    {
        return FS_MUTANT == 1 ? FS_ELEMENT : FS_ONE_PAST;
    }
    return FS_ELEMENT;
}

/* Computes a + b without executing a signed overflow.
 * Returns 1 and stores the sum in *sum, or returns 0 and leaves *sum
 * unchanged when the true sum is outside the range of int. *sum may alias
 * a or b because both are passed by value. */
static int add_checked(int a, int b, int *sum)
{
    int overflows_up = b > 0 && a > INT_MAX - b;
    int overflows_down = b < 0 && a < INT_MIN - b;

    if (overflows_up || overflows_down)
    {
        return 0;
    }
    *sum = a + b;
    return 1;
}

enum fs_status fs_sum(const int *input, size_t length, int *out)
{
    if (out == NULL || (length > 0 && input == NULL))
    {
        return FS_INVALID;
    }
    /* Accumulate privately: *out is published only after every addition
     * has succeeded, so a failure leaves the caller's object unchanged. */
    int total = 0;
    for (size_t i = 0; i < length; i++)
    {
        if (!add_checked(total, input[i], &total))
        {
            return FS_RANGE;
        }
    }
    *out = total;
    return FS_OK;
}

enum fs_status fs_row(const int (*row)[ROW_LENGTH], int *out)
{
    if (row == NULL)
    {
        return FS_INVALID;
    }
    return fs_sum(*row, ROW_LENGTH, out);
}

enum fs_status fs_matrix(const int (*rows)[ROW_LENGTH], size_t count, int *out)
{
    if (out == NULL || (count > 0 && rows == NULL))
    {
        return FS_INVALID;
    }
    /* Row-major traversal. Invariant: total is the checked sum of every
     * element before (row, column) in that order. */
    int total = 0;
    for (size_t row = 0; row < count; row++)
    {
        const size_t source_row = FS_MUTANT == 2 ? 0 : row;
        for (size_t column = 0; column < ROW_LENGTH; column++)
        {
            if (!add_checked(total, rows[source_row][column], &total))
            {
                return FS_RANGE;
            }
        }
    }
    *out = total;
    return FS_OK;
}

enum fs_status fs_branch(int branch, int *out)
{
    if (out == NULL || (branch != 0 && branch != 1))
    {
        return FS_INVALID;
    }
    /* Both paths assign before the single read below. Mutant 3 is the
     * defined-but-wrong default: it avoids an uninitialized read yet
     * implements the wrong policy. */
    int value;
    if (branch == 1)
    {
        value = 7;
    }
    else
    {
        value = FS_MUTANT == 3 ? 0 : 11;
    }
    *out = value;
    return FS_OK;
}

/* First call: appends order digit 1, increments x and returns the new x. */
static int call_first(struct fs_calls *state)
{
    state->order = state->order * ORDER_BASE + ORDER_DIGIT_FIRST;
    return ++state->x;
}

/* Second call: appends order digit 2, doubles y and returns the new y. */
static int call_second(struct fs_calls *state)
{
    state->order = state->order * ORDER_BASE + ORDER_DIGIT_SECOND;
    state->y *= 2;
    return state->y;
}

enum fs_status fs_calls(struct fs_calls *out)
{
    if (out == NULL)
    {
        return FS_INVALID;
    }
    /* Members are sum, x, y, order. The two operands of + are evaluated in
     * an unspecified order, but each call body runs without interleaving
     * with the other (indeterminately sequenced), so x, y and sum are fixed
     * and only order varies. */
    struct fs_calls stage = {0, 1, 2, 0};
    stage.sum = call_first(&stage) + call_second(&stage);
    if (FS_MUTANT == 5)
    {
        stage.sum--;
    }
    *out = stage;
    return FS_OK;
}

enum fs_status fs_comma(int *value, int *final)
{
    if (value == NULL || final == NULL || value == final)
    {
        return FS_INVALID;
    }
    /* The comma operator is a sequence point: its left operand completes
     * (i becomes 2) before the right one is evaluated and yields 2. This is
     * not the argument separator in f(i++, i++), which sequences nothing. */
    int i = 1;
    int result = (i++, i++);
    *value = FS_MUTANT == 4 ? result - 1 : result;
    *final = i;
    return FS_OK;
}
