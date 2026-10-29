#ifndef ARCHITECTURE_CACHE_H
#define ARCHITECTURE_CACHE_H
#include <stdint.h>
#include <stddef.h>
#include "algo.h"
enum ach_status { ACH_OK,ACH_INVALID,ACH_RANGE };
enum ach_category { ACH_HIT,ACH_COMPULSORY,ACH_CONFLICT,ACH_CAPACITY };
struct ach_config {unsigned lines,block_bytes,ways,shadow_fifo;};
struct ach_step {uint32_t offset,index,tag;unsigned category;uint32_t blocks[8],shadow[8];unsigned valid[8],shadow_count;};
struct ach_trace {unsigned count,hits,compulsory,conflict,capacity;struct ach_step steps[64];};
struct ach_fraction {uint32_t numerator,denominator;};
struct ach_cost_input {struct ach_fraction hit_ns,miss_ns,cpi,miss_rate;uint32_t misses,references,instructions,memory_references,penalty_cycles,frequency_hz;unsigned total_miss_latency;};
struct ach_cost {struct ach_fraction amat_ns,base_cycles,stall_cycles,total_cycles,time_ns;};
struct ach_miss {unsigned release,latency;int predecessor;};
struct ach_overlap {unsigned count,makespan,start[16],finish[16];};
enum ach_status ach_trace(const int64_t*,size_t,struct ach_config,struct ach_trace*);
enum ach_status ach_cost(struct ach_cost_input,struct ach_cost*);
enum ach_status ach_overlap(const struct ach_miss*,size_t,unsigned,struct ach_overlap*);
int ach_test(const char*);
void ach_emit(const char*,const uint32_t*,size_t);
void ach_failure(const char*,unsigned);
#endif
