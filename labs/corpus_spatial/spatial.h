#ifndef CORPUS_SPATIAL_H
#define CORPUS_SPATIAL_H
#include <stdint.h>
enum { SP_KD = 1, SP_BVH = 2, SP_QUAD = 3, SP_RTREE = 4, SP_GRID = 5 };
enum { SP_OK = 0, SP_INVALID = 1, SP_FULL = 2, SP_MISSING = 3 };
enum { SP_ITEMS = 32, SP_NODES = 128, SP_REFS = 256, SP_BUCKETS = 17 };
typedef struct {
  int32_t lo[3], hi[3];
} SpBox;
typedef struct {
  unsigned id;
  SpBox box;
} SpItem;
typedef struct {
  SpBox box;
  unsigned active, leaf, count, axis, item, child[8], entry[5];
  uint32_t mask;
} SpNode;
typedef struct {
  int32_t cell[3];
  unsigned item, next;
} SpRef;
typedef struct {
  unsigned kind, dim, count, budget, used, root, width;
  uint32_t live;
  SpItem items[SP_ITEMS];
  SpNode nodes[SP_NODES];
  SpRef refs[SP_REFS];
  unsigned buckets[SP_BUCKETS];
} SpIndex;
typedef struct {
  uint32_t mask;
  unsigned visits, tests;
} SpResult;
typedef struct {
  unsigned id, visits, tests;
  uint64_t distance;
} SpNearest;
/* All pointer arguments are live, properly aligned and mutually disjoint.
 * The input item array is immutable. scratch is a distinct actual SpIndex.
 * No pointer into a successful index survives any edit. Calls are serialized.
 * Box endpoints are closed, in [-1024,1024]; points have lo==hi.
 * In two dimensions the third coordinate is zero. IDs are unique 0..31.
 * QUAD points inhabit [-16,16)^d. Grid width is 1..16.
 * budget counts nodes (KD/BVH/QUAD/RTREE) or cell references (GRID).
 * Successful construction publishes the entire index; refusal preserves out.
 * scratch may change on failure. Empty build admits NULL items with n==0. */
int sp_build(SpIndex *out, SpIndex *scratch, unsigned kind, unsigned dim,
             const SpItem *items, unsigned n, unsigned budget, unsigned width);
/* action1 inserts missing ID, action2 replaces existing, action3 removes.
 * KD/QUAD/GRID rebuild; BVH rebuilds unless sp_refit explicitly selected.
 * RTREE performs insertion/split or deletion/condensation/reinsertion. */
int sp_edit(SpIndex *index, SpIndex *scratch, unsigned action, SpItem item);
/* BVH-only topology-preserving moved-box update, with all ancestor bounds. */
int sp_refit(SpIndex *index, SpIndex *scratch, SpItem item);
int sp_query(const SpIndex *index, SpBox query, SpResult *out);
/* KD/BVH exact weighted squared point-to-box distance; weights each 1..4.
 * Equal distance chooses smallest ID. Empty returns MISSING, output unchanged.
 */
int sp_nearest(const SpIndex *index, const int32_t point[3],
               const unsigned weights[3], SpNearest *out);
/* GRID exact overlap pairs, bits j in rows[i] iff i<j and boxes overlap.
 * Hash collisions and multiple shared cells never duplicate a pair. */
int sp_pairs(const SpIndex *index, uint32_t rows[SP_ITEMS],
             unsigned *candidates);
#endif
