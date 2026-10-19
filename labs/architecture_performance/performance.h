#ifndef CMASTERY_ARCHITECTURE_PERFORMANCE_H
#define CMASTERY_ARCHITECTURE_PERFORMANCE_H
#include <stddef.h>
#include <stdint.h>
#include "algo.h"
typedef struct {uint64_t n,d;} PERatio;
typedef struct {PERatio baseline_ns,new_ns,speedup,infinite_limit;unsigned unbounded_limit;} PEEquation;
typedef struct {uint64_t minimum,maximum,median_floor;unsigned median_half;} PEStats;
int pe_equation(uint64_t instructions,PERatio cpi,uint64_t hz,PERatio fraction,PERatio local_speed,uint64_t communication_ns,PEEquation *);
int pe_stats(const uint64_t *,size_t,PEStats *);
int pe_c(const uint32_t *,uint32_t,uint32_t,uint32_t *);
int pe_asm(const uint32_t *,uint32_t,uint32_t,uint32_t *);
#endif
