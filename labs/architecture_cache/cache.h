#ifndef ARCHITECTURE_CACHE_H
#define ARCHITECTURE_CACHE_H
#include <stdint.h>
#include <stddef.h>
#include "algo.h"
enum ACHStatus { ACH_OK,ACH_INVALID,ACH_RANGE };
enum ACHCategory { ACH_HIT,ACH_COMPULSORY,ACH_CONFLICT,ACH_CAPACITY };
typedef struct {unsigned lines,block_bytes,ways,shadow_fifo;} ACHConfig;
typedef struct {uint32_t offset,index,tag;unsigned category;uint32_t blocks[8],shadow[8];unsigned valid[8],shadow_count;} ACHStep;
typedef struct {unsigned count,hits,compulsory,conflict,capacity;ACHStep steps[64];} ACHTrace;
typedef struct {uint32_t numerator,denominator;} ACHFraction;
typedef struct {ACHFraction hit_ns,miss_ns,cpi,miss_rate;uint32_t misses,references,instructions,memory_references,penalty_cycles,frequency_hz;unsigned total_miss_latency;} ACHCostInput;
typedef struct {ACHFraction amat_ns,base_cycles,stall_cycles,total_cycles,time_ns;} ACHCost;
typedef struct {unsigned release,latency;int predecessor;} ACHMiss;
typedef struct {unsigned count,makespan,start[16],finish[16];} ACHOverlap;
enum ACHStatus ach_trace(const int64_t*,size_t,ACHConfig,ACHTrace*);
enum ACHStatus ach_cost(ACHCostInput,ACHCost*);
enum ACHStatus ach_overlap(const ACHMiss*,size_t,unsigned,ACHOverlap*);
int ach_test(const char*);
void ach_emit(const char*,const uint32_t*,size_t);
void ach_failure(const char*,unsigned);
#endif
