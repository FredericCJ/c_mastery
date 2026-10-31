/*
 * The measured kernels of the locality workload (locality.h): one pass
 * summing the key field. Each has the same source shape; only the record
 * type, hence the stride between consecutive keys, differs. Kept in their
 * own translation unit, and compiled without link-time optimization, so a
 * repeated call cannot be folded into one.
 */
#include "locality.h"

uint64_t rl_sum_soa(const struct rl_columns *columns, size_t n)
{
    const uint64_t *key = columns->key;
    uint64_t sum = 0;

    for (size_t i = 0; i < n; ++i)
    {
        sum += key[i];
    }
    return sum;
}

uint64_t rl_sum_aos16(const struct rl_record16 *records, size_t n)
{
    uint64_t sum = 0;

    for (size_t i = 0; i < n; ++i)
    {
        sum += records[i].key;
    }
    return sum;
}

uint64_t rl_sum_aos64(const struct rl_record64 *records, size_t n)
{
    uint64_t sum = 0;

    for (size_t i = 0; i < n; ++i)
    {
        sum += records[i].key;
    }
    return sum;
}

uint64_t rl_sum_aos128(const struct rl_record128 *records, size_t n)
{
    uint64_t sum = 0;

    for (size_t i = 0; i < n; ++i)
    {
        sum += records[i].key;
    }
    return sum;
}
