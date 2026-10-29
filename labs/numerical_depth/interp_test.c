#include "test.h"

int main(void)
{
    double x[] = {0, 2, 4}, y[] = {0, 4, 8}, out;
    for (int i = -1; i <= 5; i++)
        CHECK(nd_interp(x, y, 3, i, 1, &out) == ND_OK &&
              out == (i < 0   ? 0
                      : i > 4 ? 8
                              : 2 * i));
    out = 99;
    CHECK(nd_interp(x, y, 3, -1, 0, &out) == ND_RANGE && out == 99);
    CHECK(nd_interp(x, y, 0, 1, 1, &out) == ND_BAD && out == 99);
    double bad[] = {0, 0, 4};
    CHECK(nd_interp(bad, y, 3, 1, 1, &out) == ND_BAD && out == 99);
    bad[1] = 5;
    CHECK(nd_interp(bad, y, 3, 1, 1, &out) == ND_BAD && out == 99);
    CHECK(nd_interp(x, y, 3, NAN, 1, &out) == ND_BAD && out == 99);
    CHECK(nd_interp(x, y, 1, 7, 1, &out) == ND_OK && out == 0);
    out = 99;
    CHECK(nd_interp(x, y, 1, 7, 0, &out) == ND_RANGE && out == 99);
    double qx[] = {0, .5, 1}, qy[] = {0, .25, 1};
    double max = 0, at = 0;
    for (int i = 0; i <= 100; i++)
    {
        double t = i / 100.0;
        CHECK(nd_interp(qx, qy, 3, t, 1, &out) == ND_OK);
        double e = fabs(out - t * t);
        if (e > max)
        {
            max = e;
            at = t;
        }
    }
    CHECK(max == .0625 && at == .25);
    printf("PASS quadratic grid max=%a first_location=%a\n", max, at);
}
