#include "test.h"
int main(void) {
  double a[] = {0, 2, 99, 1, 3, 99}, b[] = {4, 7}, out[] = {99, 98, 97};
  struct nd_view v = {a, 6, 2, 2, 3, 1};
  struct nd_cost cost = {0};
  CHECK(nd_solve(v, b, 2, 1e-12, out, &cost) == ND_OK && out[0] == 1 &&
        out[1] == 2);
  CHECK(cost.row_moves == 1 && cost.divisions == 3 && cost.multiplies == 3 &&
        cost.adds == 3);
  double dep[] = {1, 2, 2, 4}, db[] = {3, 6};
  out[0] = 99;
  struct nd_cost old = cost;
  CHECK(nd_solve((struct nd_view){dep, 4, 2, 2, 2, 1}, db, 2, 1e-12, out, &cost) ==
            ND_SINGULAR &&
        out[0] == 99 && memcmp(&old, &cost, sizeof cost) == 0);
  for (size_t i = 0; i < 6; i++)
    if (i % 3 != 2)
      a[i] *= 1e90;
  b[0] *= 1e90;
  b[1] *= 1e90;
  CHECK(nd_solve(v, b, 2, 1e-12, out, &cost) == ND_OK &&
        fabs(out[0] - 1) < 1e-14 && fabs(out[1] - 2) < 1e-14);
  double e = 0x1p-30, near[] = {1, 1, 1, 1 + 0x1p-30},
         rhs[] = {2, 2 + 0x1p-30 + 0x1p-40};
  CHECK(nd_solve((struct nd_view){near, 4, 2, 2, 2, 1}, rhs, 2, 1e-12, out, &cost) ==
            ND_OK &&
        out[0] == 1 - 0x1p-10 && out[1] == 1 + 0x1p-10);
  (void)e;
  double aa[] = {4, 1, 1, 3}, bb[] = {1, 2};
  unsigned steps = 99;
  CHECK(nd_jacobi((struct nd_view){aa, 4, 2, 2, 2, 1}, bb, 2, 1e-12, 100, out,
                  &steps) == ND_OK &&
        fabs(out[0] - 1.0 / 11) < 1e-12 && fabs(out[1] - 7.0 / 11) < 1e-12);
  out[0] = 99;
  steps = 99;
  double diverge[] = {1, 2, 2, 1};
  CHECK(nd_jacobi((struct nd_view){diverge, 4, 2, 2, 2, 1}, bb, 2, 1e-12, 10, out,
                  &steps) == ND_LIMIT &&
        out[0] == 99 && steps == 99);
  double stagnate[] = {2, 1, 1, 3};
  CHECK(nd_jacobi((struct nd_view){stagnate, 4, 2, 2, 2, 1}, bb, 2, 0, 100, out,
                  &steps) == ND_STAGNANT &&
        out[0] == 99 && steps == 99);
  double three[] = {3, 1, 0, 99, 1, 3, 1, 99, 0, 1, 3, 99}, r3[] = {5, 10, 11};
  double explosive[] = {1, 1e100, 1e100, 1}, large_rhs[] = {1e100, 1e100};
  CHECK(nd_jacobi((struct nd_view){explosive, 4, 2, 2, 2, 1}, large_rhs, 2, 0, 3, out,
                  &steps) == ND_RANGE &&
        out[0] == 99 && steps == 99);
  CHECK(nd_solve((struct nd_view){three, 12, 3, 3, 4, 1}, r3, 3, 1e-12, out, &cost) ==
        ND_OK);
  CHECK(fabs(out[0] - 1) < 1e-14 && fabs(out[1] - 2) < 1e-14 &&
        fabs(out[2] - 3) < 1e-14);
  printf("PASS pivot, relative threshold, singular strong state, perturbation "
         "and3x3;3x3 cost=%zu,%zu,%zu,%zu\n",
         cost.adds, cost.multiplies, cost.divisions, cost.row_moves);
}
