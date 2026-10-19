#ifndef BF_SORT_H
#define BF_SORT_H
#include "depth.h"
typedef struct {size_t comparisons,writes,payload_writes;} BfWork;
typedef struct BfRecord {
 int key;
 unsigned serial;
 const struct BfRecord *self;
 unsigned char *owned;
 size_t length;
} BfRecord;
int bf_sort_width(size_t n,size_t width,size_t *next);
/* n <= 64; scalar value records; distinct typed input/scratch/work objects. */
int bf_sort_bottom(B2Item *,size_t n,B2Item *scratch,size_t capacity,BfWork *);
/* Records remain live, immutable and unmoved; only the index permutation is
 * returned. Output/scratch are distinct actual size_t arrays, each extent n.
 * This never transfers ownership or writes to records/payloads. */
int bf_sort_indirect(const BfRecord *,size_t n,size_t *indices,size_t capacity,
 size_t *scratch,size_t scratch_capacity,BfWork *);
#endif
