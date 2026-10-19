#ifndef CMASTERY_ALGORITHM_FRONTIER_H
#define CMASTERY_ALGORITHM_FRONTIER_H
#include "algorithms.h"
enum AGStatus {AG_OK,AG_INVALID,AG_SPACE,AG_RANGE,AG_BAD_CERTIFICATE};
typedef struct {uint64_t distance[8];unsigned done,seen,selected,count,frontier[8];int priority[8];} AGStep;
typedef struct {unsigned edge,from,to;uint64_t before,after;} AGRelax;
typedef struct {AGStep bfs[8],weighted[8];AGRelax relaxation[24];unsigned bfs_count,weighted_count,relaxations,queue_pushes,queue_pops,heap_inserts,heap_changes,heap_takes;uint64_t heap_comparisons;} AGTrace;
typedef struct {unsigned vertices,source,reachable,previous[8];uint32_t distance[8],hops[8];} AGResult;
typedef struct {int queue[8];CDHeap heap;CDHeapHandle handles[8];} AGWorkspace;
typedef struct {int64_t distance[8];unsigned reachable,previous[8];} AGCertificate;
enum AGStatus ag_run(const CDEdge*,size_t,unsigned,unsigned,unsigned,AGWorkspace*,size_t,AGResult*,AGTrace*);
enum AGStatus ag_certificate(const CDCSR*,unsigned,const AGCertificate*);
enum ADStatus ag_refined_bfs(const CDCSR*,unsigned,unsigned,int*,size_t,ADTraversal*,AGTrace*);
enum ADStatus ag_refined_dijkstra(const CDCSR*,unsigned,uint64_t,uint64_t[8],unsigned[8],AGWorkspace*,AGTrace*);
void ag_bfs_snapshot(AGTrace*,const ADTraversal*,unsigned,const CDRing*);
void ag_weight_snapshot(AGTrace*,const uint64_t[8],unsigned,const CDHeap*);
void ag_relax_snapshot(AGTrace*,unsigned,unsigned,unsigned,uint64_t,uint64_t);
int ag_failure(const char*,unsigned);
void ag_emit(unsigned,unsigned,const AGResult*,const AGTrace*);
int ag_test(unsigned);
#endif
