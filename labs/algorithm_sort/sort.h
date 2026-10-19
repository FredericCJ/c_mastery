#ifndef AS_SORT_H
#define AS_SORT_H
#include "algo.h"
#include <stddef.h>
#include <stdint.h>
typedef struct{uint32_t comparisons,shifts,placements,prefixes;} ASStats;
typedef struct{uint32_t used,length[32];algo_pair state[32][32];} ASPrefixes;
typedef struct{uint32_t key;uint64_t tag;} ASWide;
typedef struct{uint64_t comparisons,writes;} ASPriorCost;
_Static_assert(sizeof(algo_pair)==8,"selected original record layout");
_Static_assert(sizeof(ASWide)==16 && offsetof(ASWide,tag)==8,"selected wide record layout");
_Static_assert(sizeof(ASStats)==16 && offsetof(ASStats,prefixes)==12,"instrumentation counter layout");
typedef int(*ASCompare)(void*,uint32_t,uint32_t,int*);
extern ASStats as_stats;
extern ASPrefixes as_prefixes;
void as_prefix(const algo_pair*,uint32_t,uint32_t);
void as_instrumented_insert(algo_pair*,uint32_t);
void as_descending_insert(algo_pair*,uint32_t);
void as_wide_insert(ASWide*,uint32_t);
int as_fallible(algo_pair*,uint32_t,uint32_t,ASCompare,void*,ASStats*);
int as_prior(const algo_pair*,uint32_t,unsigned,algo_pair*,ASPriorCost*);
int as_prior_merge(const algo_pair*,uint32_t,uint32_t,algo_pair*,ASPriorCost*);
int algo_abi_insert_probe(algo_pair*,uint32_t,uint32_t,uint32_t*);
int as_wide_abi_probe(ASWide*,uint32_t,uint32_t,uint32_t*);
int as_test(unsigned);
int as_failure(const char*,unsigned);
void as_observe(unsigned,const algo_pair*,unsigned);
void as_metric(unsigned,unsigned,const ASStats*);
#endif
