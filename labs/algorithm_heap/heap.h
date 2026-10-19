#ifndef CMASTERY_ALGORITHM_HEAP_H
#define CMASTERY_ALGORITHM_HEAP_H
#include "algo.h"
#include "containers.h"
#include <stddef.h>
_Static_assert(sizeof(int)==4 && sizeof(unsigned)==4 && sizeof(CDHeapItem)==16,"selected heap assembly layout");
_Static_assert(offsetof(CDHeapItem,priority)==0 && offsetof(CDHeapItem,identity)==4 && offsetof(CDHeapItem,slot)==8 && offsetof(CDHeapItem,sequence)==12,"record offsets");
_Static_assert(offsetof(CDHeap,index)==256 && offsetof(CDHeap,count)==448 && offsetof(CDHeap,max_heap)==452 && offsetof(CDHeap,comparisons)==464,"heap offsets");
typedef struct {uint32_t comparisons,swaps;} AHStats;
typedef struct {uint32_t used,position[12],state[12][1024];} AHTrace;
extern AHStats ah_stats;
extern AHTrace ah_observed;
void ah_snapshot(const uint32_t*,uint32_t,uint32_t);
void ah_instrumented_sift(uint32_t*,uint32_t,uint32_t);
int ah_trace(uint32_t*,uint32_t,uint32_t,uint32_t,unsigned,unsigned,AHStats*,AHTrace*);
void ah_cd_sink(CDHeap*,unsigned);
void ah_heap_init(CDHeap*,unsigned);
enum CDStatus ah_heap_insert(CDHeap*,int,unsigned,CDHeapHandle*);
enum CDStatus ah_heap_take(CDHeap*,int*,unsigned*);
enum CDStatus ah_heap_get(const CDHeap*,CDHeapHandle,int*,unsigned*);
enum CDStatus ah_heap_change(CDHeap*,CDHeapHandle,int);
enum CDStatus ah_heap_build(CDHeap*,const int*,size_t,unsigned);
uint32_t ah_cd_probe(CDHeap*,unsigned,uint32_t,uint32_t*);
uint32_t algo_abi_sift_probe(uint32_t*,uint32_t,uint32_t,uint32_t*);
int ah_test(unsigned);
int ah_failure(const char*,unsigned);
void ah_emit(unsigned,const uint32_t*,unsigned,const AHStats*,const AHTrace*);
#endif
