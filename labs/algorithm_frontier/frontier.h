#ifndef CMASTERY_ALGORITHM_FRONTIER_H
#define CMASTERY_ALGORITHM_FRONTIER_H
#include "algorithms.h"
enum ag_status {AG_OK,AG_INVALID,AG_SPACE,AG_RANGE,AG_BAD_CERTIFICATE};
struct ag_step {uint64_t distance[8];unsigned done,seen,selected,count,frontier[8];int priority[8];};
struct ag_relax {unsigned edge,from,to;uint64_t before,after;};
struct ag_trace {struct ag_step bfs[8],weighted[8];struct ag_relax relaxation[24];unsigned bfs_count,weighted_count,relaxations,queue_pushes,queue_pops,heap_inserts,heap_changes,heap_takes;uint64_t heap_comparisons;};
struct ag_result {unsigned vertices,source,reachable,previous[8];uint32_t distance[8],hops[8];};
struct ag_workspace {int queue[8];struct cd_heap heap;struct cd_heap_handle handles[8];};
struct ag_certificate {int64_t distance[8];unsigned reachable,previous[8];};
enum ag_status ag_run(const struct cd_edge*,size_t,unsigned,unsigned,unsigned,struct ag_workspace*,size_t,struct ag_result*,struct ag_trace*);
enum ag_status ag_certificate(const struct cd_csr*,unsigned,const struct ag_certificate*);
enum ad_status ag_refined_bfs(const struct cd_csr*,unsigned,unsigned,int*,size_t,struct ad_traversal*,struct ag_trace*);
enum ad_status ag_refined_dijkstra(const struct cd_csr*,unsigned,uint64_t,uint64_t[8],unsigned[8],struct ag_workspace*,struct ag_trace*);
void ag_bfs_snapshot(struct ag_trace*,const struct ad_traversal*,unsigned,const struct cd_ring*);
void ag_weight_snapshot(struct ag_trace*,const uint64_t[8],unsigned,const struct cd_heap*);
void ag_relax_snapshot(struct ag_trace*,unsigned,unsigned,unsigned,uint64_t,uint64_t);
int ag_failure(const char*,unsigned);
void ag_emit(unsigned,unsigned,const struct ag_result*,const struct ag_trace*);
int ag_test(unsigned);
#endif
