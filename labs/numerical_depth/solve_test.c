#include "test.h"

#define SENTINEL 99

/*
 * 2x2 system with a zero leading pivot, in a buffer with padding (row stride
 * 3). The exact solution is (1, 2); the cost counters are the operation counts
 * of that solve.
 */
static void check_pivot_singular_and_scaling(void)
{
    double a[] = {0, 2, 99, 1, 3, 99};
    double b[] = {4, 7};
    double out[] = {SENTINEL, 98, 97};
    struct nd_view view = {a, 6, 2, 2, 3, 1};
    struct nd_cost cost = {0, 0, 0, 0};
    CHECK(nd_solve(view, b, 2, 1e-12, out, &cost) == ND_OK && out[0] == 1 &&
          out[1] == 2);
    CHECK(cost.row_moves == 1 && cost.divisions == 3 && cost.multiplies == 3 &&
          cost.adds == 3);

    /* Dependent rows: rejected, with output and cost both unchanged. */
    double dependent[] = {1, 2, 2, 4};
    double dependent_rhs[] = {3, 6};
    out[0] = SENTINEL;
    struct nd_cost before = cost;
    CHECK(nd_solve((struct nd_view){dependent, 4, 2, 2, 2, 1}, dependent_rhs, 2,
                   1e-12, out, &cost) == ND_SINGULAR &&
          out[0] == SENTINEL && memcmp(&before, &cost, sizeof cost) == 0);

    /* Scaling by 1e90 stays inside the admitted 1e100 bound; still solvable. */
    for (size_t i = 0; i < 6; i++)
    {
        if (i % 3 != 2)
        {
            a[i] *= 1e90;
        }
    }
    b[0] *= 1e90;
    b[1] *= 1e90;
    CHECK(nd_solve(view, b, 2, 1e-12, out, &cost) == ND_OK &&
          fabs(out[0] - 1) < 1e-14 && fabs(out[1] - 2) < 1e-14);
}

/* Rows that differ by 2^-30: the rounded answer is checked bit for bit. */
static void check_near_dependent_rows(void)
{
    double near_rows[] = {1, 1, 1, 1 + 0x1p-30};
    double rhs[] = {2, 2 + 0x1p-30 + 0x1p-40};
    double out[3];
    struct nd_cost cost;
    CHECK(nd_solve((struct nd_view){near_rows, 4, 2, 2, 2, 1}, rhs, 2, 1e-12,
                   out, &cost) == ND_OK &&
          out[0] == 1 - 0x1p-10 && out[1] == 1 + 0x1p-10);
}

/*
 * Jacobi iteration: convergence on a diagonally dominant system, then the
 * three failure modes, each of which must leave output and step count alone.
 */
static void check_jacobi(void)
{
    double dominant[] = {4, 1, 1, 3};
    double rhs[] = {1, 2};
    double out[3];
    unsigned steps = SENTINEL;
    CHECK(nd_jacobi((struct nd_view){dominant, 4, 2, 2, 2, 1}, rhs, 2, 1e-12,
                    100, out, &steps) == ND_OK &&
          fabs(out[0] - 1.0 / 11) < 1e-12 && fabs(out[1] - 7.0 / 11) < 1e-12);

    out[0] = SENTINEL;
    steps = SENTINEL;
    double diverging[] = {1, 2, 2, 1};
    CHECK(nd_jacobi((struct nd_view){diverging, 4, 2, 2, 2, 1}, rhs, 2, 1e-12,
                    10, out, &steps) == ND_LIMIT &&
          out[0] == SENTINEL && steps == SENTINEL);

    /* Zero tolerance: only an exactly zero residual meets the stopping rule. */
    double stagnating[] = {2, 1, 1, 3};
    CHECK(nd_jacobi((struct nd_view){stagnating, 4, 2, 2, 2, 1}, rhs, 2, 0, 100,
                    out, &steps) == ND_STAGNANT &&
          out[0] == SENTINEL && steps == SENTINEL);

    double explosive[] = {1, 1e100, 1e100, 1};
    double large_rhs[] = {1e100, 1e100};
    CHECK(nd_jacobi((struct nd_view){explosive, 4, 2, 2, 2, 1}, large_rhs, 2, 0,
                    3, out, &steps) == ND_RANGE &&
          out[0] == SENTINEL && steps == SENTINEL);
}

/* 3x3 tridiagonal system (row stride 4) with solution (1, 2, 3). */
static void check_three_by_three(struct nd_cost *cost)
{
    double system[] = {3, 1, 0, 99, 1, 3, 1, 99, 0, 1, 3, 99};
    double rhs[] = {5, 10, 11};
    double out[3];
    CHECK(nd_solve((struct nd_view){system, 12, 3, 3, 4, 1}, rhs, 3, 1e-12, out,
                   cost) == ND_OK);
    CHECK(fabs(out[0] - 1) < 1e-14 && fabs(out[1] - 2) < 1e-14 &&
          fabs(out[2] - 3) < 1e-14);
}

int main(void)
{
    struct nd_cost cost3 = {0, 0, 0, 0};
    check_pivot_singular_and_scaling();
    check_near_dependent_rows();
    check_jacobi();
    check_three_by_three(&cost3);
    printf(
        "PASS pivot, relative threshold, singular strong state, perturbation "
        "and3x3;3x3 cost=%zu,%zu,%zu,%zu\n",
        cost3.adds, cost3.multiplies, cost3.divisions, cost3.row_moves);
}
