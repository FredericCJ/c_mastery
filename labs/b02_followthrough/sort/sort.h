#ifndef BF_SORT_H
#define BF_SORT_H
#include "depth.h"
struct bf_work {size_t comparisons,writes,payload_writes;};
struct bf_record {
 int key;
 unsigned serial;
 const struct bf_record *self;
 unsigned char *owned;
 size_t length;
};
int bf_sort_width(size_t n,size_t width,size_t *next);
/* n <= 64; scalar value records; distinct typed input/scratch/work objects. */
int bf_sort_bottom(struct b2_item *,size_t n,struct b2_item *scratch,size_t capacity,struct bf_work *);
/* Records remain live, immutable and unmoved; only the index permutation is
 * returned. Output/scratch are distinct actual size_t arrays, each extent n.
 * This never transfers ownership or writes to records/payloads. */
int bf_sort_indirect(const struct bf_record *,size_t n,size_t *indices,size_t capacity,
 size_t *scratch,size_t scratch_capacity,struct bf_work *);
#endif
