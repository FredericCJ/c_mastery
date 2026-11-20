/*
 * Representation/locality workload. Contract: locality.h.
 */
#include "locality.h"

#include <stdlib.h>
#include <string.h>

static const char *const layout_names[RL_LAYOUT_COUNT] = {
    [RL_SOA] = "soa",
    [RL_AOS16] = "aos16",
    [RL_AOS64] = "aos64",
    [RL_AOS128] = "aos128",
};

static const size_t layout_strides[RL_LAYOUT_COUNT] = {
    [RL_SOA] = sizeof(uint64_t),
    [RL_AOS16] = sizeof(struct rl_record16),
    [RL_AOS64] = sizeof(struct rl_record64),
    [RL_AOS128] = sizeof(struct rl_record128),
};

bool rl_layout_lookup(const char *name, enum rl_layout *layout)
{
    for (unsigned i = 0; (name != NULL) && (i < RL_LAYOUT_COUNT); ++i)
    {
        if (strcmp(name, layout_names[i]) == 0)
        {
            *layout = (enum rl_layout)i;
            return true;
        }
    }
    return false;
}

const char *rl_layout_name(enum rl_layout layout)
{
    return ((unsigned)layout < RL_LAYOUT_COUNT) ? layout_names[layout]
                                                : "invalid";
}

size_t rl_stride_bytes(enum rl_layout layout)
{
    return ((unsigned)layout < RL_LAYOUT_COUNT) ? layout_strides[layout] : 0u;
}

/* A deterministic cold word: the measured loop never reads it, but every
 * byte is written so each page is backed before any measurement. */
static uint64_t cold_word(uint64_t seed, size_t record, size_t field)
{
    return seed ^ ((uint64_t)record * UINT64_C(0x9E3779B97F4A7C15)) ^
           ((uint64_t)field << 56);
}

/* Page-aligned storage for `bytes` (rounded up to whole pages); n is
 * bounded by RL_MAX_N, so no size computation here can wrap. */
static void *allocate(size_t bytes, size_t *total)
{
    size_t rounded =
        (bytes + RL_PAGE_BYTES - 1u) / RL_PAGE_BYTES * RL_PAGE_BYTES;
    void *block = aligned_alloc(RL_PAGE_BYTES, rounded);

    if (block != NULL)
    {
        *total += rounded;
    }
    return block;
}

static enum rl_status create_columns(size_t n, uint64_t seed,
                                     struct rl_data *data)
{
    struct rl_columns *columns = &data->columns;

    columns->key = allocate(n * sizeof(uint64_t), &data->bytes);
    bool complete = (columns->key != NULL);
    for (size_t j = 0; j < RL_COLD_COLUMNS; ++j)
    {
        columns->cold[j] = allocate(n * sizeof(uint64_t), &data->bytes);
        complete = complete && (columns->cold[j] != NULL);
    }
    if (!complete)
    {
        return RL_NO_MEMORY;
    }
    for (size_t i = 0; i < n; ++i)
    {
        columns->key[i] = 2u * (uint64_t)i;
        for (size_t j = 0; j < RL_COLD_COLUMNS; ++j)
        {
            columns->cold[j][i] = cold_word(seed, i, j);
        }
    }
    return RL_OK;
}

static enum rl_status create_records(size_t n, uint64_t seed,
                                     struct rl_data *data)
{
    size_t stride = layout_strides[data->layout];

    data->records = allocate(n * stride, &data->bytes);
    if (data->records == NULL)
    {
        return RL_NO_MEMORY;
    }
    /* Written through the record type itself, field by field. */
    switch (data->layout)
    {
        case RL_AOS16:
        {
            struct rl_record16 *records = data->records;

            for (size_t i = 0; i < n; ++i)
            {
                records[i].key = 2u * (uint64_t)i;
                records[i].cold[0] = cold_word(seed, i, 0);
            }
            break;
        }
        case RL_AOS64:
        {
            struct rl_record64 *records = data->records;

            for (size_t i = 0; i < n; ++i)
            {
                records[i].key = 2u * (uint64_t)i;
                for (size_t j = 0; j < 7u; ++j)
                {
                    records[i].cold[j] = cold_word(seed, i, j);
                }
            }
            break;
        }
        case RL_AOS128:
        default:
        {
            struct rl_record128 *records = data->records;

            for (size_t i = 0; i < n; ++i)
            {
                records[i].key = 2u * (uint64_t)i;
                for (size_t j = 0; j < 15u; ++j)
                {
                    records[i].cold[j] = cold_word(seed, i, j);
                }
            }
            break;
        }
    }
    return RL_OK;
}

enum rl_status rl_create(enum rl_layout layout, size_t n, uint64_t seed,
                         struct rl_data *data)
{
    if ((data == NULL) || ((unsigned)layout >= RL_LAYOUT_COUNT) || (n == 0u) ||
        (n > RL_MAX_N))
    {
        return RL_INVALID;
    }
    memset(data, 0, sizeof *data);
    data->layout = layout;
    data->n = n;
    enum rl_status status = (layout == RL_SOA) ? create_columns(n, seed, data)
                                               : create_records(n, seed, data);
    if (status != RL_OK)
    {
        rl_destroy(data);
    }
    return status;
}

void rl_destroy(struct rl_data *data)
{
    if (data == NULL)
    {
        return;
    }
    free(data->columns.key);
    for (size_t j = 0; j < RL_COLD_COLUMNS; ++j)
    {
        free(data->columns.cold[j]);
    }
    free(data->records);
    memset(data, 0, sizeof *data);
}

uint64_t rl_sum(const struct rl_data *data, unsigned repetitions)
{
    uint64_t total = 0;

    for (unsigned r = 0; r < repetitions; ++r)
    {
        switch (data->layout)
        {
            case RL_SOA:
                total += rl_sum_soa(&data->columns, data->n);
                break;
            case RL_AOS16:
                total += rl_sum_aos16(data->records, data->n);
                break;
            case RL_AOS64:
                total += rl_sum_aos64(data->records, data->n);
                break;
            case RL_AOS128:
            default:
                total += rl_sum_aos128(data->records, data->n);
                break;
        }
    }
    return total;
}

uint64_t rl_oracle(size_t n, unsigned repetitions)
{
    /* Sum of 2i for i < n is n(n-1); unsigned arithmetic wraps exactly as
     * the kernels' sums do. */
    uint64_t per_pass = (uint64_t)n * ((uint64_t)n - 1u);

    return per_pass * repetitions;
}
