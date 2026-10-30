/*
 * Reference direct and iterative solvers for 2x2 and 3x3 systems.
 *
 * The system is copied into a local augmented matrix m = [A | b] (n rows,
 * n + 1 columns), so the caller's view and right-hand side are never
 * modified. Results and operation counts are published only on success; every
 * failure return leaves out, cost and steps untouched.
 *
 * nd_solve is Gaussian elimination with row-scaled partial pivoting.
 * nd_jacobi starts from zero and reads only the previous iterate in each
 * sweep. All arithmetic is binary64 in the order written; the counters tally
 * additions, multiplications and divisions of elimination and
 * back-substitution only (not admission checks, pivot comparisons or swaps).
 *
 * Mutant ND_MUTANT == 5 disables pivoting (always uses the diagonal).
 */
#include "numeric.h"
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif

enum
{
    AUGMENTED_COLUMNS = ND_SOLVE_MAX_ORDER + 1
};

/*
 * Copies a and b into the augmented matrix m and records in scale[i] the
 * largest magnitude in row i of A. Returns 1, or 0 when the view is invalid,
 * the order is not 2..ND_SOLVE_MAX_ORDER, b is null, or an entry is not finite
 * or exceeds ND_VALUE_LIMIT.
 */
static int stage_system(struct nd_view a, const double *b, size_t n,
                        double m[][AUGMENTED_COLUMNS], double scale[])
{
    if (nd_view(a) || !b || n < 2 || n > ND_SOLVE_MAX_ORDER || a.rows != n ||
        a.cols != n)
    {
        return 0;
    }
    for (size_t i = 0; i < n; i++)
    {
        scale[i] = 0;
        for (size_t j = 0; j < n; j++)
        {
            double x = a.data[i * a.rs + j * a.cs];
            if (!isfinite(x) || fabs(x) > ND_VALUE_LIMIT)
            {
                return 0;
            }
            m[i][j] = x;
            if (fabs(x) > scale[i])
            {
                scale[i] = fabs(x);
            }
        }
        if (!isfinite(b[i]) || fabs(b[i]) > ND_VALUE_LIMIT)
        {
            return 0;
        }
        m[i][n] = b[i];
    }
    return 1;
}

/*
 * Row of the pivot for column k: the largest |m[i][k]| / scale[i] over rows
 * k..n-1, lowest row on ties. A row with a zero scale is returned only when no
 * other row qualifies; the caller then reports the matrix as singular.
 */
static size_t choose_pivot(double m[][AUGMENTED_COLUMNS], const double scale[],
                           size_t n, size_t k)
{
    size_t pivot = k;
    for (size_t i = k + 1; i < n; i++)
    {
        if (scale[i] && (!scale[pivot] || fabs(m[i][k]) / scale[i] >
                                              fabs(m[pivot][k]) / scale[pivot]))
        {
            pivot = i;
        }
    }
    if (ND_MUTANT == 5)
    {
        pivot = k;
    }
    return pivot;
}

static void swap_rows(double m[][AUGMENTED_COLUMNS], double scale[], size_t n,
                      size_t p, size_t k)
{
    for (size_t j = 0; j <= n; j++)
    {
        double entry = m[p][j];
        m[p][j] = m[k][j];
        m[k][j] = entry;
    }
    double row_scale = scale[p];
    scale[p] = scale[k];
    scale[k] = row_scale;
}

/*
 * Zeroes column k below the diagonal by subtracting multiples of row k.
 * Returns ND_RANGE as soon as any updated entry is not finite, else ND_OK.
 */
static int eliminate_column(double m[][AUGMENTED_COLUMNS], size_t n, size_t k,
                            struct nd_cost *cost)
{
    for (size_t i = k + 1; i < n; i++)
    {
        double factor = m[i][k] / m[k][k];
        cost->divisions++;
        m[i][k] = 0;
        for (size_t j = k + 1; j <= n; j++)
        {
            m[i][j] -= factor * m[k][j];
            cost->multiplies++;
            cost->adds++;
            if (!isfinite(m[i][j]))
            {
                return ND_RANGE;
            }
        }
    }
    return ND_OK;
}

/* Solves the upper-triangular system left by elimination, last row first. */
static int back_substitute(double m[][AUGMENTED_COLUMNS], size_t n, double x[],
                           struct nd_cost *cost)
{
    for (size_t row_plus_one = n; row_plus_one; row_plus_one--)
    {
        size_t i = row_plus_one - 1;
        double rest = m[i][n];
        for (size_t j = i + 1; j < n; j++)
        {
            rest -= m[i][j] * x[j];
            cost->multiplies++;
            cost->adds++;
        }
        x[i] = rest / m[i][i];
        cost->divisions++;
        if (!isfinite(x[i]))
        {
            return ND_RANGE;
        }
    }
    return ND_OK;
}

int nd_solve(struct nd_view a, const double *b, size_t n, double tolerance,
             double *out, struct nd_cost *cost)
{
    double m[ND_SOLVE_MAX_ORDER][AUGMENTED_COLUMNS];
    double scale[ND_SOLVE_MAX_ORDER];
    double x[ND_SOLVE_MAX_ORDER];
    struct nd_cost counted = {0, 0, 0, 0};
    if (!out || !cost || !isfinite(tolerance) || tolerance < 0 ||
        tolerance >= 1 || !stage_system(a, b, n, m, scale))
    {
        return ND_BAD;
    }
    for (size_t k = 0; k < n; k++)
    {
        size_t pivot = choose_pivot(m, scale, n, k);
        /* Relative screen: a diagnostic policy, not a conditioning proof. */
        if (!scale[pivot] || fabs(m[pivot][k]) <= tolerance * scale[pivot])
        {
            return ND_SINGULAR;
        }
        if (pivot != k)
        {
            swap_rows(m, scale, n, pivot, k);
            counted.row_moves++;
        }
        int status = eliminate_column(m, n, k, &counted);
        if (status != ND_OK)
        {
            return status;
        }
    }
    int status = back_substitute(m, n, x, &counted);
    if (status != ND_OK)
    {
        return status;
    }
    for (size_t i = 0; i < n; i++)
    {
        out[i] = x[i];
    }
    *cost = counted;
    return ND_OK;
}

/*
 * One Jacobi sweep, reading only x:
 *   next[i] = (b[i] - sum over j != i of A[i][j] * x[j]) / A[i][i].
 * Sets *unchanged to 1 when next equals x exactly. ND_RANGE on a nonfinite
 * entry.
 */
static int jacobi_sweep(double m[][AUGMENTED_COLUMNS], size_t n,
                        const double x[], double next[], int *unchanged)
{
    *unchanged = 1;
    for (size_t i = 0; i < n; i++)
    {
        double rest = m[i][n];
        for (size_t j = 0; j < n; j++)
        {
            if (i != j)
            {
                rest -= m[i][j] * x[j];
            }
        }
        next[i] = rest / m[i][i];
        if (!isfinite(next[i]))
        {
            return ND_RANGE;
        }
        if (next[i] != x[i])
        {
            *unchanged = 0;
        }
    }
    return ND_OK;
}

/* Infinity norm of A * next - b. ND_RANGE if any component is not finite. */
static int residual_norm(double m[][AUGMENTED_COLUMNS], size_t n,
                         const double next[], double *norm)
{
    *norm = 0;
    for (size_t i = 0; i < n; i++)
    {
        double residual = -m[i][n];
        for (size_t j = 0; j < n; j++)
        {
            residual += m[i][j] * next[j];
        }
        if (!isfinite(residual))
        {
            return ND_RANGE;
        }
        if (fabs(residual) > *norm)
        {
            *norm = fabs(residual);
        }
    }
    return ND_OK;
}

int nd_jacobi(struct nd_view a, const double *b, size_t n, double tol,
              unsigned limit, double *out, unsigned *steps)
{
    double m[ND_SOLVE_MAX_ORDER][AUGMENTED_COLUMNS];
    double scale[ND_SOLVE_MAX_ORDER];
    double x[ND_SOLVE_MAX_ORDER] = {0};
    double next[ND_SOLVE_MAX_ORDER];
    if (!out || !steps || !isfinite(tol) || tol < 0 ||
        limit > ND_JACOBI_LIMIT_MAX || !stage_system(a, b, n, m, scale))
    {
        return ND_BAD;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (m[i][i] == 0)
        {
            return ND_SINGULAR;
        }
    }
    for (unsigned k = 0; k < limit; k++)
    {
        int unchanged;
        int status = jacobi_sweep(m, n, x, next, &unchanged);
        if (status != ND_OK)
        {
            return status;
        }
        double norm;
        status = residual_norm(m, n, next, &norm);
        if (status != ND_OK)
        {
            return status;
        }
        if (norm <= tol)
        {
            for (size_t i = 0; i < n; i++)
            {
                out[i] = next[i];
            }
            *steps = k + 1;
            return ND_OK;
        }
        if (unchanged)
        {
            return ND_STAGNANT;
        }
        for (size_t i = 0; i < n; i++)
        {
            x[i] = next[i];
        }
    }
    return ND_LIMIT;
}
