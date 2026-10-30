/*
 * Driver that feeds test cases to the reference and prints its answers for an
 * independent exact-rational oracle to judge. Usage: oracle_driver CASEFILE.
 *
 * CASEFILE is text: repeated records of a count n (at most MAX_CASE_ELEMENTS)
 * followed by n pairs of hexadecimal floating-point literals "a b". For each
 * record the driver prints "index naive kahan neumaier dot" in hexadecimal
 * floating point (%a, exact and round-trippable). Exit status: 0 success, 2
 * usage or open error, 3 malformed or unreadable input, 4 reference refused a
 * case.
 */
#include "float_contract.h"
#include <math.h>
#include <stdio.h>

#define MAX_CASE_ELEMENTS 4096u
#define SUM_METHOD_COUNT 3
#define RESULT_COUNT 4 /* three sums plus the dot product */

/* Read one record's pairs into a[] and b[]. Returns 0 on success, 3 on a
 * malformed record. */
static int read_pairs(FILE *input, size_t n, double *a, double *b)
{
    for (size_t i = 0; i < n; i++)
    {
        if (fscanf(input, "%la %la", &a[i], &b[i]) != 2)
        {
            return 3;
        }
    }
    return 0;
}

/* Compute the three sums and the dot product. Returns 0 or 4. */
static int run_reference(const double *a, const double *b, size_t n,
                         double results[RESULT_COUNT])
{
    for (int i = 0; i < SUM_METHOD_COUNT; i++)
    {
        if (ff_sum(a, n, (enum ff_method)i, &results[i]) != FF_OK)
        {
            return 4;
        }
    }
    if (ff_dot(a, b, n, &results[SUM_METHOD_COUNT]) != FF_OK)
    {
        return 4;
    }
    return 0;
}

/* Process every record of an open stream; the caller owns and closes it.
 * Returns 0 or the exit status for the first failure. */
static int process_cases(FILE *input, size_t *case_count)
{
    size_t n = 0;
    double a[MAX_CASE_ELEMENTS];
    double b[MAX_CASE_ELEMENTS];
    while (fscanf(input, "%zu", &n) == 1)
    {
        if (n > MAX_CASE_ELEMENTS)
        {
            return 3;
        }
        int status = read_pairs(input, n, a, b);
        double results[RESULT_COUNT];
        if (status == 0)
        {
            status = run_reference(a, b, n, results);
        }
        if (status != 0)
        {
            return status;
        }
        printf("%zu %a %a %a %a\n", (*case_count)++, results[0], results[1],
               results[2], results[3]);
    }
    return ferror(input) ? 3 : 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        return 2;
    }

    size_t case_count = 0;
    int status = process_cases(input, &case_count);
    if (fclose(input) != 0)
    {
        status = (status != 0) ? status : 3;
    }
    if (status != 0)
    {
        return status;
    }
    fprintf(stderr, "FF_ORACLE_DRIVER_PASS cases=%zu\n", case_count);
    return 0;
}
