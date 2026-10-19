#ifndef BF_REGION_H
#define BF_REGION_H
#include "bridge.h"
#include "arena.h"
typedef struct BfCohort BfCohort;
typedef struct { _Alignas(32) unsigned value; unsigned id,live,opens,closes;BfCohort*owner;} BfObject;
struct BfCohort {MBRegion bytes;BfObject objects[4];unsigned trace[16],used;};
void bf_cohort_init(BfCohort *);
int bf_cohort_batch(BfCohort *,const unsigned char *,size_t,unsigned,unsigned,unsigned *);
int bf_cohort_reset(BfCohort *);
typedef struct {MBRegion parent,child[2];} BfRegionTree;
void bf_tree_init(BfRegionTree *);
int bf_tree_child_reset(BfRegionTree *,unsigned);
int bf_tree_reset(BfRegionTree *);
typedef struct {unsigned char*base;size_t begin,capacity,cursor,payload,high;unsigned generation;} BfSlice;
typedef struct {const BfSlice*owner;unsigned generation;size_t offset,length;} BfSliceSpan;
int bf_slice_init(BfSlice *,unsigned char *,size_t,size_t,size_t);
int bf_slice_alloc(BfSlice *,size_t,size_t,BfSliceSpan *);
int bf_slice_read(const BfSlice *,BfSliceSpan,size_t,unsigned char *);
int bf_slice_reset(BfSlice *);
typedef struct {_Alignas(8) unsigned char bytes[32];size_t cursor;} BfChunk;
typedef BfChunk *(*BfAcquireChunk)(void *);
typedef void (*BfReleaseChunk)(void *,BfChunk *);
typedef struct {
 BfChunk*chunks[4];size_t count,payload,occupied,high,capacity_high;
 unsigned generation,busy;BfAcquireChunk acquire;BfReleaseChunk release;void*context;
} BfGrowth;
typedef struct {const BfGrowth*owner;unsigned generation;size_t chunk,offset,length;} BfGrowthSpan;
int bf_growth_init(BfGrowth *,BfChunk *,BfAcquireChunk,BfReleaseChunk,void *);
int bf_growth_alloc(BfGrowth *,size_t,size_t,BfGrowthSpan *);
int bf_growth_read(const BfGrowth *,BfGrowthSpan,size_t,unsigned char *);
int bf_growth_reset(BfGrowth *);
typedef struct {BfArena arena;B2Mark mark;unsigned active;} BfRetryRegion;
void bf_retry_init(BfRetryRegion *);
int bf_retry_reserve(BfRetryRegion *,size_t,size_t,BfBorrow *);
int bf_retry_register(BfRetryRegion *,BfClose,void *);
int bf_retry_reset(BfRetryRegion *);
#endif
