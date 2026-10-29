#define _POSIX_C_SOURCE 200809L
#include "dod_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <time.h>

static uint64_t now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        exit(2);
    }
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}

static uint64_t row_sum(const struct d_row *p, size_t n)
{
    uint64_t sum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        sum += (uint32_t)p[i].x + (uint64_t)(uint32_t)p[i].velocity + p[i].cold;
    }
    return sum;
}

static uint64_t col_sum(const int32_t *x, const int32_t *v, const unsigned *c,
                        size_t n)
{
    uint64_t sum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        sum += (uint32_t)x[i] + (uint64_t)(uint32_t)v[i] + c[i];
    }
    return sum;
}

static uint64_t row_index(const struct d_row *p, const size_t *indices,
                          size_t n)
{
    uint64_t sum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        size_t j = indices[i];
        sum += (uint32_t)p[j].x + (uint64_t)(uint32_t)p[j].velocity + p[j].cold;
    }
    return sum;
}

static uint64_t col_index(const int32_t *x, const int32_t *v, const unsigned *c,
                          const size_t *indices, size_t n)
{
    uint64_t sum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        size_t j = indices[i];
        sum += (uint32_t)x[j] + (uint64_t)(uint32_t)v[j] + c[j];
    }
    return sum;
}

int main(void)
{
    const size_t sizes[] = {1024, 65536, 1048576};
    const size_t eviction_size = 64u * 1024u * 1024u;
    volatile unsigned char *eviction = malloc(eviction_size);
    if (!eviction)
    {
        return 2;
    }
    volatile unsigned observed = 0;
    unsigned rng = 12345;
    for (unsigned size = 0; size < 3; ++size)
    {
        size_t n = sizes[size];
        struct d_row *rows = malloc(n * sizeof(*rows));
        int32_t *x = malloc(n * sizeof(*x)), *v = malloc(n * sizeof(*v));
        unsigned *cold = malloc(n * sizeof(*cold));
        size_t *indices = malloc(n * sizeof(*indices));
        if (!rows || !x || !v || !cold || !indices)
        {
            return 2;
        }
        for (size_t i = 0; i < n; ++i)
        {
            rows[i] =
                (struct d_row){(int32_t)(i % 1000), 2, (unsigned)(i % 251)};
            x[i] = rows[i].x;
            v[i] = 2;
            cold[i] = rows[i].cold;
        }
        if (row_sum(rows, n) != col_sum(x, v, cold, n))
        {
            return 3;
        }
        for (size_t i = 0; i < n; ++i)
        {
            indices[i] = i;
        }
        for (size_t i = n; i > 1; --i)
        {
            rng = rng * 1664525u + 1013904223u;
            size_t j = rng % i, tmp = indices[i - 1];
            indices[i - 1] = indices[j];
            indices[j] = tmp;
        }
        for (unsigned condition = 0; condition < 2; ++condition)
        {
            for (unsigned trial = 0; trial < 7; ++trial)
            {
                unsigned order[] = {0, 1, 2, 3, 4, 5, 6, 7};
                for (unsigned i = 8; i > 1; --i)
                {
                    rng = rng * 1664525u + 1013904223u;
                    unsigned j = rng % i, tmp = order[i - 1];
                    order[i - 1] = order[j];
                    order[j] = tmp;
                }
                for (unsigned ix = 0; ix < 8; ++ix)
                {
                    unsigned kind = order[ix];
                    for (size_t i = 0; i < n; ++i)
                    {
                        rows[i].x = (int32_t)(i % 1000);
                        x[i] = rows[i].x;
                    }
                    if (kind == 6)
                    {
                        for (size_t i = 0; i < n; ++i)
                        {
                            x[i] = -777;
                            v[i] = -778;
                            cold[i] = 999;
                        }
                    }
                    if (kind == 7)
                    {
                        for (size_t i = 0; i < n; ++i)
                        {
                            rows[i] = (struct d_row){-779, -780, 998};
                        }
                    }
                    if (condition)
                    {
                        for (size_t i = 0; i < eviction_size; i += 64)
                        {
                            eviction[i] = (unsigned char)i;
                            observed += eviction[i];
                        }
                    }
                    atomic_signal_fence(memory_order_seq_cst);
                    uint64_t start = now(), checksum = 0;
                    int status = 0;
                    if (kind == 0)
                    {
                        status = d_rows_step_many(rows, n, 2);
                    }
                    else if (kind == 1)
                    {
                        status = d_columns_step_many(x, v, n, 2);
                    }
                    else if (kind == 2)
                    {
                        checksum = row_sum(rows, n);
                    }
                    else if (kind == 3)
                    {
                        checksum = col_sum(x, v, cold, n);
                    }
                    else if (kind == 4)
                    {
                        checksum = row_index(rows, indices, n);
                    }
                    else if (kind == 5)
                    {
                        checksum = col_index(x, v, cold, indices, n);
                    }
                    else if (kind == 6)
                    {
                        status = d_rows_to_arrays(rows, n, x, v, cold, n);
                    }
                    else
                    {
                        status = d_arrays_to_rows(x, v, cold, n, rows, n);
                    }
                    atomic_signal_fence(memory_order_seq_cst);
                    uint64_t elapsed = now() - start;
                    if (status)
                    {
                        return 4;
                    }
                    if (kind < 2)
                    {
                        checksum = 0;
                        for (size_t i = 0; i < n; ++i)
                        {
                            int32_t expected = (int32_t)(i % 1000) + 4,
                                    value = kind == 0 ? rows[i].x : x[i];
                            if (value != expected)
                            {
                                return 5;
                            }
                            checksum += (uint32_t)value;
                        }
                    }
                    if (kind >= 6)
                    {
                        for (size_t i = 0; i < n; ++i)
                        {
                            if (rows[i].x != x[i] || rows[i].velocity != v[i] ||
                                rows[i].cold != cold[i])
                            {
                                return 7;
                            }
                        }
                        checksum = row_sum(rows, n);
                    }
                    printf(
                        "{\"n\":%zu,\"eviction_conditioned\":%u,\"trial\":%u,\"kind\":%u,\"nanoseconds\":%llu,\"checksum\":%llu}\n",
                        n, condition, trial, kind, (unsigned long long)elapsed,
                        (unsigned long long)checksum);
                }
            }
        }
        free(rows);
        free(x);
        free(v);
        free(cold);
        free(indices);
    }
    free((void *)eviction);
    return observed == UINT32_MAX ? 6 : 0;
}
