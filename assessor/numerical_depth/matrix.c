#include "numeric.h"
#include <math.h>
#include <stdint.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif
int nd_view(struct nd_view v)
{
    if (!v.data || !v.rows || !v.cols || v.rows > 8 || v.cols > 8 || !v.rs ||
        !v.cs)
    {
        return ND_BAD;
    }
    if (v.rows - 1 > SIZE_MAX / v.rs || v.cols - 1 > SIZE_MAX / v.cs)
    {
        return ND_BAD;
    }
    size_t a = (v.rows - 1) * v.rs, b = (v.cols - 1) * v.cs;
    if (a > SIZE_MAX - b || a + b >= v.extent)
    {
        return ND_BAD;
    }
    for (size_t i = 0; i < v.rows; i++)
    {
        for (size_t j = 0; j < v.cols; j++)
        {
            for (size_t k = 0; k < v.rows; k++)
            {
                for (size_t l = 0; l < v.cols; l++)
                {
                    if ((i != k || j != l) &&
                        i * v.rs + j * v.cs == k * v.rs + l * v.cs)
                    {
                        return ND_BAD;
                    }
                }
            }
        }
    }
    return ND_OK;
}

static int valid(struct nd_view v)
{
    if (nd_view(v))
    {
        return 0;
    }
    for (size_t i = 0; i < v.rows; i++)
    {
        for (size_t j = 0; j < v.cols; j++)
        {
            if (!isfinite(v.data[i * v.rs + j * v.cs]) ||
                fabs(v.data[i * v.rs + j * v.cs]) > 1e100)
            {
                return 0;
            }
        }
    }
    return 1;
}

static int overlap(struct nd_view a, struct nd_view b)
{
    for (size_t i = 0; i < a.rows; i++)
    {
        for (size_t j = 0; j < a.cols; j++)
        {
            for (size_t k = 0; k < b.rows; k++)
            {
                for (size_t l = 0; l < b.cols; l++)
                {
                    if (&a.data[i * a.rs + j * a.cs] ==
                        &b.data[k * b.rs + l * b.cs])
                    {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int nd_matrix(struct nd_view a, struct nd_view b, struct nd_view c,
              unsigned method)
{
    if (!valid(a) || !valid(b) || nd_view(c) || a.cols != b.rows ||
        c.rows != a.rows || c.cols != b.cols || method > 2 || overlap(a, c) ||
        overlap(b, c))
    {
        return ND_BAD;
    }
    double t[64] = {0}, bt[64];
    if (method == 1)
    {
        for (size_t k = 0; k < b.rows; k++)
        {
            for (size_t j = 0; j < b.cols; j++)
            {
                bt[j * b.rows + k] = b.data[k * b.rs + j * b.cs];
            }
        }
    }
    size_t block = method == 2 ? 2 : 8;
    for (size_t ib = 0; ib < a.rows; ib += block)
    {
        for (size_t jb = 0; jb < b.cols; jb += block)
        {
            for (size_t i = ib; i < a.rows && i < ib + block; i++)
            {
                for (size_t j = jb; j < b.cols && j < jb + block; j++)
                {
                    double s = 0;
                    for (size_t k = 0; k < a.cols; k++)
                    {
                        double av = a.data[i * a.rs + k * a.cs],
                               bv = method == 1 ? bt[j * b.rows + k]
                                                : b.data[k * b.rs + j * b.cs];
                        s += av * bv;
                    }
                    t[i * c.cols + j] = s;
                }
            }
        }
    }
    for (size_t i = 0; i < c.rows; i++)
    {
        for (size_t j = 0; j < c.cols; j++)
        {
            c.data[ND_MUTANT == 3 ? i * c.cols + j : i * c.rs + j * c.cs] =
                t[i * c.cols + j];
        }
    }
    return ND_OK;
}

int nd_scale(struct nd_view v, double s)
{
    if (!valid(v) || !isfinite(s) || fabs(s) > 1e100)
    {
        return ND_BAD;
    }
    for (size_t i = 0; i < v.rows; i++)
    {
        for (size_t j = 0; j < v.cols; j++)
        {
            v.data[i * v.rs + j * v.cs] *= s;
        }
    }
    return ND_OK;
}

int nd_dot(struct nd_view a, struct nd_view b, double *out)
{
    if (!out || !valid(a) || !valid(b) || a.rows != 1 || b.rows != 1 ||
        a.cols != b.cols)
    {
        return ND_BAD;
    }
    double s = 0;
    for (size_t j = 0; j < a.cols; j++)
    {
        s += a.data[j * a.cs] * b.data[j * b.cs];
    }
    *out = s;
    return ND_OK;
}
