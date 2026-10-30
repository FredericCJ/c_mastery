/*
 * Neighbour-distance boundaries for ff_ulps: at each seed (zero, smallest
 * normal, tiny, one, huge, DBL_MAX) of both signs, the next double up and down
 * is exactly one unit away, except where nextafter leaves the finite range, in
 * which case ff_ulps must refuse and leave its output alone. Exit status: 0
 * pass, 1 contract violation, 2 environment save/restore failure.
 */
#include "float_contract.h"
#include <float.h>
#include <math.h>
#include <stdio.h>

#define UNTOUCHED_DISTANCE 917u

/* Check one double neighbour pair; returns 0 on agreement with the contract. */
static int check_double_pair(size_t partition, int sign, int direction,
                             double x)
{
    double y = nextafter(x, (direction < 0) ? -INFINITY : INFINITY);
    uint64_t distance = UNTOUCHED_DISTANCE;
    enum ff_status status = ff_ulps(x, y, &distance);
    if (isfinite(y))
    {
        if ((status != FF_OK) || (distance != 1u))
        {
            return 1;
        }
    }
    else if ((status != FF_RANGE) || (distance != UNTOUCHED_DISTANCE))
    {
        return 1;
    }
    printf("BOUNDARY partition=%zu sign=%d direction=%d x=%a y=%a finite=%d\n",
           partition, sign, direction, x, y, !!isfinite(y));
    return 0;
}

static int check_double_boundaries(unsigned *pairs)
{
    const double seeds[] = {0, DBL_MIN, 0x1p-100, 1, 0x1p100, DBL_MAX};
    for (size_t i = 0; i < sizeof seeds / sizeof seeds[0]; i++)
    {
        for (int sign = -1; sign <= 1; sign += 2)
        {
            for (int direction = -1; direction <= 1; direction += 2)
            {
                double x = sign * seeds[i];
                if (check_double_pair(i, sign, direction, x) != 0)
                {
                    return 1;
                }
                (*pairs)++;
            }
        }
    }
    return 0;
}

/* Single-precision neighbours are printed for the dossier (as doubles, exact)
 * and only required to be distinct; ff_ulps itself is a binary64 contract. */
static int check_float_neighbours(unsigned *pairs)
{
    const float seeds[] = {0, FLT_MIN, 1, FLT_MAX};
    for (size_t i = 0; i < sizeof seeds / sizeof seeds[0]; i++)
    {
        for (int direction = -1; direction <= 1; direction += 2)
        {
            float x = seeds[i];
            float y = nextafterf(x, (direction < 0) ? -INFINITY : INFINITY);
            printf(
                "FLOAT_BOUNDARY partition=%zu direction=%d x=%a y=%a precision=%d\n",
                i, direction, (double)x, (double)y, FLT_MANT_DIG);
            (*pairs)++;
            if (x == y)
            {
                return 1;
            }
        }
    }
    return 0;
}

int main(void)
{
    fenv_t saved;
    if ((fegetenv(&saved) != 0) || (fesetround(FE_TONEAREST) != 0))
    {
        return 2;
    }

    unsigned double_pairs = 0;
    unsigned float_pairs = 0;
    if (check_double_boundaries(&double_pairs) != 0)
    {
        return 1;
    }
    if (check_float_neighbours(&float_pairs) != 0)
    {
        return 1;
    }
    if (fesetenv(&saved) != 0)
    {
        return 2;
    }
    printf(
        "FF_BOUNDARY_PASS double_pairs=%u float_pairs=%u algorithmic_neighbors\n",
        double_pairs, float_pairs);
    return 0;
}
