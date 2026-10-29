#include "test.h"
#include <fenv.h>

int main(void)
{
    CHECK(fegetround() == FE_TONEAREST);
    double a[] = {0x1p53, 1, -0x1p53}, o = 37;
    double expected[] = {0, 1, 1, 1};
    for (unsigned m = 0; m < 4; m++)
    {
        CHECK(nd_sum(a, 3, m, &o) == ND_OK);
        CHECK(o == expected[m]);
        CHECK(nd_sum(NULL, 0, m, &o) == ND_OK && o == 0);
    }
    double b[] = {1e16, 1, -1e16};
    CHECK(nd_sum(b, 3, ND_KAHAN, &o) == ND_OK && o == 0);
    CHECK(nd_sum(b, 3, ND_NEUMAIER, &o) == ND_OK && o == 1);
    double zero[] = {1, -1}, bad[] = {1, NAN};
    CHECK(nd_sum(zero, 2, 3, &o) == ND_OK && o == 0);
    o = 37;
    CHECK(nd_sum(bad, 2, 3, &o) == ND_BAD && o == 37);
    CHECK(nd_sum(a, 4097, 0, &o) == ND_BAD && o == 37);
    double x[4096];
    for (size_t i = 0; i < 4096; i++)
    {
        x[i] = 1;
    }
    for (unsigned m = 0; m < 4; m++)
    {
        CHECK(nd_sum(x, 4096, m, &o) == ND_OK && o == 4096);
    }
    puts("PASS sums and nonfinite strong output");
}
