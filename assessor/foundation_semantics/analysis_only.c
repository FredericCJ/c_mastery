/* NEVER LINK OR EXECUTE. Warnings are observations, not a semantic oracle. */
#include <stddef.h>

int fs_off_by_one(const int *p, size_t n)
{
    int sum = 0;
    for (size_t i = 0; i <= n; i++)
        sum += p[i];
    return sum;
}

int fs_uninitialized(void)
{
    int value;
    return value;
}

int fs_partial_init(int branch)
{
    int value;
    if (branch)
        value = 7;
    return value;
}

int fs_conflicting(void)
{
    int i = 1;
    return i++ + i++;
}

int fs_add(int a, int b)
{
    return a + b;
}

int fs_argument_commas(void)
{
    int i = 1;
    return fs_add(i++, i++);
}
