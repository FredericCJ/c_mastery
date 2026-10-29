#include "performance.h"
#ifndef BP_MUTANT
#define BP_MUTANT 0
#endif
void bf_add(const uint32_t *a, const uint32_t *b, uint32_t *out, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = a[i] + b[i] + (BP_MUTANT == 1);
    }
}

void bf_add_disjoint(const uint32_t *restrict a, const uint32_t *restrict b,
                     uint32_t *restrict out, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = a[i] + b[i];
    }
}

void bf_add_inplace(uint32_t *a, const uint32_t *b, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        a[i] += b[i];
    }
}

void bf_add_unroll(const uint32_t *a, const uint32_t *b, uint32_t *out,
                   size_t n, unsigned lanes)
{
    size_t i = 0;
    if (lanes != 1 && lanes != 2 && lanes != 4 && lanes != 8)
    {
        return;
    }
    for (; n - i >= lanes; i += lanes)
    {
        for (unsigned j = 0; j < lanes; ++j)
        {
            out[i + j] = a[i + j] + b[i + j];
        }
    }
    if (BP_MUTANT == 2)
    {
        return;
    }
    for (; i < n; ++i)
    {
        out[i] = a[i] + b[i];
    }
}

int bf_select(int feature, int os_support, int force_scalar)
{
    return !force_scalar && feature > 0 && os_support;
}

void bf_dispatch(const uint32_t *a, const uint32_t *b, uint32_t *o, size_t n,
                 int feature, int os_support, int force_scalar)
{
    if (bf_select(feature, os_support, force_scalar))
    {
        bf_sse2(a, b, o, n);
    }
    else
    {
        bf_add(a, b, o, n);
    }
}

int bf_clamp_branch(int x)
{
    if (x < 0)
    {
        return 0;
    }
    if (x > 4)
    {
        return 4;
    }
    return x;
}

int bf_clamp_select(int x)
{
    int lo = x < 0 ? 0 : x;
    return lo > 4 ? 4 : lo;
}

uint32_t bf_recurrence(const uint32_t *a, size_t n)
{
    uint32_t r = 0;
    for (size_t i = 0; i < n; ++i)
    {
        r = r * 33u + a[i];
    }
    return r;
}

float bf_reduce(const float *a, size_t n)
{
    float r = 0.0f;
    for (size_t i = 0; i < n; ++i)
    {
        r += a[i];
    }
    return r;
}

void bf_to_soa(const struct bf_record *in, size_t n, uint32_t *id,
               uint32_t *value)
{
    for (size_t i = 0; i < n; ++i)
    {
        id[i] = in[i].id;
        value[i] = in[i].value;
    }
}

int bf_delete_soa(uint32_t *id, uint32_t *value, size_t *n, uint32_t remove_id)
{
    for (size_t i = 0; i < *n; ++i)
    {
        if (id[i] == remove_id)
        {
            for (size_t j = i; j + 1 < *n; ++j)
            {
                id[j] = id[j + 1];
                value[j] = value[j + 1];
            }
            --*n;
            return 1;
        }
    }
    return 0;
}

void bf_blocked(const uint32_t *in, size_t rows, size_t cols, size_t stride,
                uint32_t *out, size_t out_stride, size_t block)
{
    if (!block)
    {
        return;
    }
    for (size_t rr = 0; rr < rows; rr += block)
    {
        for (size_t cc = 0; cc < cols; cc += block)
        {
            for (size_t r = rr; r < rows && r - rr < block; ++r)
            {
                for (size_t c = cc; c < cols && c - cc < block; ++c)
                {
                    out[c * out_stride + r] = in[r * stride + c];
                }
            }
        }
    }
}

#define BF_UNROLL(N)                                                     \
    void bf_unroll##N(const uint32_t *a, const uint32_t *b, uint32_t *o, \
                      size_t n)                                          \
    {                                                                    \
        bf_add_unroll(a, b, o, n, N);                                    \
    }
BF_UNROLL(1) BF_UNROLL(2) BF_UNROLL(4) BF_UNROLL(8)
