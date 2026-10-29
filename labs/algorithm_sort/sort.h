#ifndef AS_SORT_H
#define AS_SORT_H
#include "algo.h"
#include <stddef.h>
#include <stdint.h>
struct as_stats{uint32_t comparisons,shifts,placements,prefixes;};
struct as_prefixes{uint32_t used,length[32];algo_pair state[32][32];};
struct as_wide{uint32_t key;uint64_t tag;};
struct as_prior_cost{uint64_t comparisons,writes;};
_Static_assert(sizeof(algo_pair)==8,"selected original record layout");
_Static_assert(sizeof(struct as_wide)==16 && offsetof(struct as_wide,tag)==8,"selected wide record layout");
_Static_assert(sizeof(struct as_stats)==16 && offsetof(struct as_stats,prefixes)==12,"instrumentation counter layout");
typedef int(*as_compare_fn)(void*,uint32_t,uint32_t,int*);
extern struct as_stats as_stats;
extern struct as_prefixes as_prefixes;
void as_prefix(const algo_pair*,uint32_t,uint32_t);
void as_instrumented_insert(algo_pair*,uint32_t);
void as_descending_insert(algo_pair*,uint32_t);
void as_wide_insert(struct as_wide*,uint32_t);
int as_fallible(algo_pair*,uint32_t,uint32_t,as_compare_fn,void*,struct as_stats*);
int as_prior(const algo_pair*,uint32_t,unsigned,algo_pair*,struct as_prior_cost*);
int as_prior_merge(const algo_pair*,uint32_t,uint32_t,algo_pair*,struct as_prior_cost*);
int algo_abi_insert_probe(algo_pair*,uint32_t,uint32_t,uint32_t*);
int as_wide_abi_probe(struct as_wide*,uint32_t,uint32_t,uint32_t*);
int as_test(unsigned);
int as_failure(const char*,unsigned);
void as_observe(unsigned,const algo_pair*,unsigned);
void as_metric(unsigned,unsigned,const struct as_stats*);
#endif
