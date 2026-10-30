/* CM-SPECIMEN: undefined behaviour (out-of-bounds read, uninitialized read,
 * partially initialized read, unsequenced modification, unspecified
 * argument evaluation order).
 *
 * NEVER LINK OR EXECUTE. These functions are translated with -c only.
 * Compiler warnings are observations, not a semantic oracle. */
#include <stddef.h>

/* Reads p[n]: the loop bound should be i < n. */
int fs_off_by_one(const int *p, size_t n)
{
    int sum = 0;
    for (size_t i = 0; i <= n; i++)
    {
        sum += p[i];
    }
    return sum;
}

/* Reads an automatic object that was never assigned. */
int fs_uninitialized(void)
{
    int value;
    return value;
}

/* The branch == 0 path reaches the return without any assignment. */
int fs_partial_init(int branch)
{
    int value;
    if (branch)
    {
        value = 7;
    }
    return value;
}

/* Two unsequenced modifications of i in one expression. */
int fs_conflicting(void)
{
    int i = 1;
    return i++ + i++;
}

int fs_add(int a, int b)
{
    return a + b;
}

/* Argument separators sequence nothing: the two i++ conflict. */
int fs_argument_commas(void)
{
    int i = 1;
    return fs_add(i++, i++);
}
