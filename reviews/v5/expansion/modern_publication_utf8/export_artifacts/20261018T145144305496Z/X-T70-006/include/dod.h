#ifndef CMASTERY_DOD_H
#define CMASTERY_DOD_H
#include <stddef.h>
#include <stdint.h>
enum DStatus {D_OK,D_INVALID,D_FULL,D_RANGE,D_STALE,D_FAILED};
typedef struct {void *context;unsigned char *(*allocate)(void *,size_t);void (*release)(void *,unsigned char *);} DAllocator;
typedef struct {unsigned char local[16];unsigned char *heap;size_t length,capacity;DAllocator allocator;} DSmall;
void d_small_init(DSmall *,DAllocator);
int d_small_append(DSmall *,const unsigned char *,size_t);
int d_small_truncate(DSmall *,size_t);
int d_small_compact(DSmall *);
int d_small_move(DSmall *destination,DSmall *source);
void d_small_reset(DSmall *);
const unsigned char *d_small_data(const DSmall *);
typedef struct {int32_t x[4],velocity[4];unsigned cold[4];size_t count;} DColumns;
int d_columns_step(DColumns *,int32_t ticks);
uint32_t d_swar_add(uint32_t,uint32_t);
typedef struct {unsigned generation,live;int value;} DSlot;
typedef struct {DSlot slots[4];unsigned dense[4],rank[4],count;} DSparse;
typedef struct {const DSparse *owner;unsigned slot,generation;} DHandle;
void d_sparse_init(DSparse *);
int d_sparse_add(DSparse *,int,DHandle *);
int d_sparse_get(const DSparse *,DHandle,int *);
int d_sparse_remove(DSparse *,DHandle);
typedef struct {unsigned ids[2],count;int position[2],velocity[2];} DTable;
typedef struct {DTable tables[3];unsigned mask[4],row[4];} DEcs;
void d_ecs_init(DEcs *);
int d_ecs_set(DEcs *,unsigned id,unsigned mask,int position,int velocity);
int d_ecs_step(DEcs *);
typedef struct {unsigned left,right;int value;} DTreeNode;
typedef struct {DTreeNode nodes[65];unsigned used,fail_after,allocations;} DTree;
void d_tree_init(DTree *);
int d_tree_update(DTree *,unsigned root,unsigned index,int value,unsigned *new_root);
int d_tree_get(const DTree *,unsigned root,unsigned index,int *);
typedef struct {unsigned bitmap,children[4],count,keys[2];int values[2];} DHamtNode;
typedef struct {DHamtNode nodes[65];unsigned used,fail_after,allocations;} DHamt;
void d_hamt_init(DHamt *);
int d_hamt_update(DHamt *,unsigned root,unsigned key,int value,unsigned *new_root);
int d_hamt_get(const DHamt *,unsigned root,unsigned key,int *);
typedef struct {unsigned left,right,length;unsigned char bytes[8];} DRopeNode;
typedef struct {DRopeNode nodes[65];unsigned used;} DRope;
void d_rope_init(DRope *);
int d_rope_leaf(DRope *,const unsigned char *,size_t,unsigned *);
int d_rope_concat(DRope *,unsigned,unsigned,unsigned *);
int d_rope_flatten(const DRope *,unsigned,unsigned char *,size_t,size_t *);
#endif
