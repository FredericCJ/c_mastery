/* CM-SPECIMEN: signed overflow, division by zero, unrepresentable quotient
 * (INT_MIN / -1) and normalization overflow, each in an unguarded function.
 *
 * Compile only. No portable golden result and no execution requirement: the
 * functions are never linked into a program or called, because calling them
 * with the dossier's arguments is undefined behaviour (SEI CERT INT32-C,
 * INT33-C). Their guarded counterparts are in integer.c. */
#include <limits.h>

int fi_bad_add(int a, int b)
{
    return a + b;
}

int fi_bad_divide(int a, int b)
{
    return a / b;
}

int fi_bad_remainder(int a, int b)
{
    return a % b;
}

int fi_bad_normalize(int a, int m)
{
    int r = a % m;
    return (r + m) % m;
}

/* Caller contracts above are deliberately absent: signed overflow, divisor0,
 * unrepresentable quotient and normalization overflow are dossier cases. */
