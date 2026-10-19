#ifndef CMASTERY_DOD_ROPE_STATS_H
#define CMASTERY_DOD_ROPE_STATS_H
#include "dod_text.h"
typedef struct {unsigned depth,logical_bytes,reachable_nodes,unique_leaves,stored_bytes,leaf_slack,retained_unreachable,fibonacci_min,balanced;} DRopeStats;
int d_rope_stats(const DRope *,unsigned root,DRopeStats *);
#endif
