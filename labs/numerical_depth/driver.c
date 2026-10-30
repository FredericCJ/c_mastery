/*
 * Line-oriented oracle driver. Reads commands from stdin, calls the numerical
 * functions and prints one result line per command: the status, then (on
 * ND_OK) every output as a C99 hexadecimal floating-point value ("%a"), which
 * round-trips binary64 exactly.
 *
 * Commands (all numbers are whitespace separated; reals are hexadecimal):
 *   s METHOD N X1..XN              nd_sum
 *   i N CLAMP QUERY (X Y)*N        nd_interp
 *   e Y0 H RATE N                  nd_euler
 *   g N TOLERANCE A(N*N) B(N)      nd_solve, n = 2 or 3, row-major A
 *
 * Exit status 2 for an unknown command, a malformed or out-of-range field.
 * The driver does not check the nd_* functions' outputs for correctness; the
 * Python oracle does.
 */
#include "numeric.h"
#include <stdio.h>

enum
{
    DRIVER_MALFORMED = 2,
    OUT_CAPACITY = ND_STREAM_MAX + 1,
    SOLVE_STORAGE = ND_SOLVE_MAX_ORDER * ND_SOLVE_MAX_ORDER
};

/* One command's status and its output values (count of them, all doubles). */
struct driver_result
{
    int status;
    size_t count;
    double out[OUT_CAPACITY];
};

/* Reads n hexadecimal doubles into dest. Returns 1 on success, 0 otherwise. */
static int read_doubles(double *dest, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (scanf("%la", &dest[i]) != 1)
        {
            return 0;
        }
    }
    return 1;
}

static int command_sum(struct driver_result *result)
{
    unsigned method;
    size_t n;
    double x[ND_SUM_MAX];
    if (scanf("%u%zu", &method, &n) != 2 || n > ND_SUM_MAX ||
        !read_doubles(x, n))
    {
        return DRIVER_MALFORMED;
    }
    result->status = nd_sum(x, n, method, result->out);
    return 0;
}

static int command_interp(struct driver_result *result)
{
    size_t n;
    int clamp;
    double query;
    double x[ND_INTERP_MAX];
    double y[ND_INTERP_MAX];
    if (scanf("%zu%d%la", &n, &clamp, &query) != 3 || n > ND_INTERP_MAX)
    {
        return DRIVER_MALFORMED;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (scanf("%la%la", x + i, y + i) != 2)
        {
            return DRIVER_MALFORMED;
        }
    }
    result->status = nd_interp(x, y, n, query, clamp, result->out);
    return 0;
}

static int command_euler(struct driver_result *result)
{
    double y0;
    double h;
    double rate;
    size_t n;
    if (scanf("%la%la%la%zu", &y0, &h, &rate, &n) != 4 || n > ND_STREAM_MAX)
    {
        return DRIVER_MALFORMED;
    }
    result->status = nd_euler(y0, h, rate, n, result->out, OUT_CAPACITY);
    result->count = n + 1;
    return 0;
}

static int command_solve(struct driver_result *result)
{
    size_t n;
    double tolerance;
    double a[SOLVE_STORAGE];
    double b[ND_SOLVE_MAX_ORDER];
    struct nd_cost cost;
    if (scanf("%zu%la", &n, &tolerance) != 2 || n < 2 ||
        n > ND_SOLVE_MAX_ORDER || !read_doubles(a, n * n) ||
        !read_doubles(b, n))
    {
        return DRIVER_MALFORMED;
    }
    struct nd_view view = {a, n * n, n, n, n, 1};
    result->status = nd_solve(view, b, n, tolerance, result->out, &cost);
    result->count = n;
    return 0;
}

static void print_result(const struct driver_result *result)
{
    printf("%d", result->status);
    if (result->status == ND_OK)
    {
        for (size_t i = 0; i < result->count; i++)
        {
            printf(" %a", result->out[i]);
        }
    }
    putchar('\n');
}

int main(void)
{
    char op;
    while (scanf(" %c", &op) == 1)
    {
        struct driver_result result = {ND_BAD, 1, {0}};
        int failure;
        if (op == 's')
        {
            failure = command_sum(&result);
        }
        else if (op == 'i')
        {
            failure = command_interp(&result);
        }
        else if (op == 'e')
        {
            failure = command_euler(&result);
        }
        else if (op == 'g')
        {
            failure = command_solve(&result);
        }
        else
        {
            failure = DRIVER_MALFORMED;
        }
        if (failure)
        {
            return failure;
        }
        print_result(&result);
    }
    return 0;
}
