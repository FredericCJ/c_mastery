/*
 * Reference strided views and small matrix operations.
 *
 * A view addresses element (i, j) at data[i * rs + j * cs]. Views are at most
 * ND_VIEW_DIM_MAX by ND_VIEW_DIM_MAX, so every staging array below holds 64
 * doubles. Admission (shape, bounds, overlap, value range) is done before the
 * first write, so a rejected call changes nothing. All three multiply methods
 * accumulate each element over k in increasing order with a separate multiply
 * and add, so they return identical bits; they differ only in traversal and
 * scratch use.
 *
 * Mutant ND_MUTANT == 3 writes the product with the staging layout instead of
 * the output view's strides.
 */
#include "numeric.h"
#include <math.h>
#include <stdint.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif

enum
{
    STAGING_DOUBLES = ND_VIEW_DIM_MAX * ND_VIEW_DIM_MAX,
    /* One tile as large as any view: plain row-by-row traversal. */
    BLOCK_ROW_ORDER = ND_VIEW_DIM_MAX,
    BLOCK_TILED = 2,
    /* Method numbers accepted by nd_matrix; 0 is the plain traversal. */
    METHOD_TRANSPOSED = 1,
    METHOD_TILED = 2
};

/* Offset, in elements, of logical element (i, j). Callers pass indices that
 * nd_view has already checked, so the arithmetic cannot overflow. */
static size_t element_offset(struct nd_view v, size_t i, size_t j)
{
    return i * v.rs + j * v.cs;
}

/* 1 when the largest offset is computable and lies inside extent. */
static int view_fits_extent(struct nd_view v)
{
    if (v.rows - 1 > SIZE_MAX / v.rs || v.cols - 1 > SIZE_MAX / v.cs)
    {
        return 0;
    }
    size_t last_row = (v.rows - 1) * v.rs;
    size_t last_col = (v.cols - 1) * v.cs;
    if (last_row > SIZE_MAX - last_col)
    {
        return 0;
    }
    return last_row + last_col < v.extent;
}

/* 1 when no two distinct logical elements share an offset. */
static int view_offsets_distinct(struct nd_view v)
{
    for (size_t i = 0; i < v.rows; i++)
    {
        for (size_t j = 0; j < v.cols; j++)
        {
            for (size_t k = 0; k < v.rows; k++)
            {
                for (size_t l = 0; l < v.cols; l++)
                {
                    int same_element = i == k && j == l;
                    if (!same_element &&
                        element_offset(v, i, j) == element_offset(v, k, l))
                    {
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

int nd_view(struct nd_view v)
{
    if (!v.data || !v.rows || !v.cols || v.rows > ND_VIEW_DIM_MAX ||
        v.cols > ND_VIEW_DIM_MAX || !v.rs || !v.cs)
    {
        return ND_BAD;
    }
    if (!view_fits_extent(v) || !view_offsets_distinct(v))
    {
        return ND_BAD;
    }
    return ND_OK;
}

/* 1 when v is a valid view whose elements are all finite and within limit. */
static int view_admitted(struct nd_view v)
{
    if (nd_view(v))
    {
        return 0;
    }
    for (size_t i = 0; i < v.rows; i++)
    {
        for (size_t j = 0; j < v.cols; j++)
        {
            double element = v.data[element_offset(v, i, j)];
            if (!isfinite(element) || fabs(element) > ND_VALUE_LIMIT)
            {
                return 0;
            }
        }
    }
    return 1;
}

/* 1 when some logical element of a and some of b are the same object. */
static int views_overlap(struct nd_view a, struct nd_view b)
{
    for (size_t i = 0; i < a.rows; i++)
    {
        for (size_t j = 0; j < a.cols; j++)
        {
            for (size_t k = 0; k < b.rows; k++)
            {
                for (size_t l = 0; l < b.cols; l++)
                {
                    if (&a.data[element_offset(a, i, j)] ==
                        &b.data[element_offset(b, k, l)])
                    {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/* Row-major transpose of b into bt, so bt[j * b.rows + k] is element (k, j). */
static void stage_transpose(struct nd_view b, double *bt)
{
    for (size_t k = 0; k < b.rows; k++)
    {
        for (size_t j = 0; j < b.cols; j++)
        {
            bt[j * b.rows + k] = b.data[element_offset(b, k, j)];
        }
    }
}

/* One element of a * b: sum over k, in increasing k, of a(i,k) * b(k,j). When
 * bt is not NULL, b's elements are read from the staged transpose instead. */
static double product_element(struct nd_view a, struct nd_view b,
                              const double *bt, size_t i, size_t j)
{
    double sum = 0;
    for (size_t k = 0; k < a.cols; k++)
    {
        double av = a.data[element_offset(a, i, k)];
        double bv = bt ? bt[j * b.rows + k] : b.data[element_offset(b, k, j)];
        sum += av * bv;
    }
    return sum;
}

/*
 * Computes every element of a * b into product (row-major, a.rows by b.cols),
 * visiting the output in square tiles of `block` rows and columns. A tile
 * that runs past the edge is clipped, so sizes that do not divide evenly work.
 */
static void multiply_by_tiles(struct nd_view a, struct nd_view b,
                              const double *bt, size_t block, double *product)
{
    for (size_t ib = 0; ib < a.rows; ib += block)
    {
        for (size_t jb = 0; jb < b.cols; jb += block)
        {
            for (size_t i = ib; i < a.rows && i < ib + block; i++)
            {
                for (size_t j = jb; j < b.cols && j < jb + block; j++)
                {
                    product[i * b.cols + j] = product_element(a, b, bt, i, j);
                }
            }
        }
    }
}

int nd_matrix(struct nd_view a, struct nd_view b, struct nd_view c,
              unsigned method)
{
    if (!view_admitted(a) || !view_admitted(b) || nd_view(c) ||
        a.cols != b.rows || c.rows != a.rows || c.cols != b.cols ||
        method > METHOD_TILED || views_overlap(a, c) || views_overlap(b, c))
    {
        return ND_BAD;
    }
    /* The product is staged so c is written only after all reads are done. */
    double product[STAGING_DOUBLES] = {0};
    double transposed_b[STAGING_DOUBLES];
    const double *bt = NULL;
    if (method == METHOD_TRANSPOSED)
    {
        stage_transpose(b, transposed_b);
        bt = transposed_b;
    }
    size_t block = method == METHOD_TILED ? BLOCK_TILED : BLOCK_ROW_ORDER;
    multiply_by_tiles(a, b, bt, block, product);
    for (size_t i = 0; i < c.rows; i++)
    {
        for (size_t j = 0; j < c.cols; j++)
        {
            size_t destination =
                ND_MUTANT == 3 ? i * c.cols + j : element_offset(c, i, j);
            c.data[destination] = product[i * c.cols + j];
        }
    }
    return ND_OK;
}

int nd_scale(struct nd_view v, double s)
{
    if (!view_admitted(v) || !isfinite(s) || fabs(s) > ND_VALUE_LIMIT)
    {
        return ND_BAD;
    }
    for (size_t i = 0; i < v.rows; i++)
    {
        for (size_t j = 0; j < v.cols; j++)
        {
            v.data[element_offset(v, i, j)] *= s;
        }
    }
    return ND_OK;
}

int nd_dot(struct nd_view a, struct nd_view b, double *out)
{
    if (!out || !view_admitted(a) || !view_admitted(b) || a.rows != 1 ||
        b.rows != 1 || a.cols != b.cols)
    {
        return ND_BAD;
    }
    double sum = 0;
    for (size_t j = 0; j < a.cols; j++)
    {
        sum += a.data[j * a.cs] * b.data[j * b.cs];
    }
    *out = sum;
    return ND_OK;
}
