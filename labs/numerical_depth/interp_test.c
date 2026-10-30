#include "test.h"

#define SENTINEL 99

/* Knots at 0, 2, 4 on the line y = 2x, queried from -1 to 5 in clamp mode. */
static void check_clamped_line(void)
{
    double x[] = {0, 2, 4};
    double y[] = {0, 4, 8};
    double out;
    for (int query = -1; query <= 5; query++)
    {
        double expected = 2 * query;
        if (query < 0)
        {
            expected = 0;
        }
        else if (query > 4)
        {
            expected = 8;
        }
        CHECK(nd_interp(x, y, 3, query, 1, &out) == ND_OK && out == expected);
    }
}

static void check_rejections_keep_output(void)
{
    double x[] = {0, 2, 4};
    double y[] = {0, 4, 8};
    double out = SENTINEL;
    CHECK(nd_interp(x, y, 3, -1, 0, &out) == ND_RANGE && out == SENTINEL);
    CHECK(nd_interp(x, y, 0, 1, 1, &out) == ND_BAD && out == SENTINEL);
    double unsorted[] = {0, 0, 4};
    CHECK(nd_interp(unsorted, y, 3, 1, 1, &out) == ND_BAD && out == SENTINEL);
    unsorted[1] = 5;
    CHECK(nd_interp(unsorted, y, 3, 1, 1, &out) == ND_BAD && out == SENTINEL);
    CHECK(nd_interp(x, y, 3, NAN, 1, &out) == ND_BAD && out == SENTINEL);
}

static void check_single_knot(void)
{
    double x[] = {0, 2, 4};
    double y[] = {0, 4, 8};
    double out = SENTINEL;
    CHECK(nd_interp(x, y, 1, 7, 1, &out) == ND_OK && out == 0);
    out = SENTINEL;
    CHECK(nd_interp(x, y, 1, 7, 0, &out) == ND_RANGE && out == SENTINEL);
}

/*
 * Samples y = x^2 through three knots at 101 points. The largest observed
 * error is reported with the first query that attains it.
 */
static void check_quadratic_grid(void)
{
    const double x[] = {0, .5, 1};
    const double y[] = {0, .25, 1};
    double out;
    double max_error = 0;
    double max_at = 0;
    for (int i = 0; i <= 100; i++)
    {
        double t = i / 100.0;
        CHECK(nd_interp(x, y, 3, t, 1, &out) == ND_OK);
        double error = fabs(out - t * t);
        if (error > max_error)
        {
            max_error = error;
            max_at = t;
        }
    }
    CHECK(max_error == .0625 && max_at == .25);
    printf("PASS quadratic grid max=%a first_location=%a\n", max_error, max_at);
}

int main(void)
{
    check_clamped_line();
    check_rejections_keep_output();
    check_single_knot();
    check_quadratic_grid();
}
