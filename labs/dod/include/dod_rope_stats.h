#ifndef CMASTERY_DOD_ROPE_STATS_H
#define CMASTERY_DOD_ROPE_STATS_H
#include "dod_text.h"
struct d_rope_stats {unsigned depth,logical_bytes,reachable_nodes,unique_leaves,stored_bytes,leaf_slack,retained_unreachable,fibonacci_min,balanced;};
int d_rope_stats(const struct d_rope *,unsigned root,struct d_rope_stats *);
#endif
