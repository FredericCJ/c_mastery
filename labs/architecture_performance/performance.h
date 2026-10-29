#ifndef CMASTERY_ARCHITECTURE_PERFORMANCE_H
#define CMASTERY_ARCHITECTURE_PERFORMANCE_H
#include <stddef.h>
#include <stdint.h>
#include "algo.h"

struct pe_ratio
{
    uint64_t n, d;
};

struct pe_equation
{
    struct pe_ratio baseline_ns, new_ns, speedup, infinite_limit;
    unsigned unbounded_limit;
};

struct pe_stats
{
    uint64_t minimum, maximum, median_floor;
    unsigned median_half;
};

int pe_equation(uint64_t instructions, struct pe_ratio cpi, uint64_t hz,
                struct pe_ratio fraction, struct pe_ratio local_speed,
                uint64_t communication_ns, struct pe_equation *);
int pe_stats(const uint64_t *, size_t, struct pe_stats *);
int pe_c(const uint32_t *, uint32_t, uint32_t, uint32_t *);
int pe_asm(const uint32_t *, uint32_t, uint32_t, uint32_t *);
#endif
