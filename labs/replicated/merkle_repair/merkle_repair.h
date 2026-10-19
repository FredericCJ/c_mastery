#ifndef CMASTERY_REPLICATED_MERKLE_REPAIR_H
#define CMASTERY_REPLICATED_MERKLE_REPAIR_H
#include <stddef.h>
#include <stdint.h>
enum {
  RPK_OK,
  RPK_INVALID,
  RPK_FULL,
  RPK_PROOF,
  RPK_PROTOCOL,
  RPK_MISSING,
  RPK_UNIMPLEMENTED
};
typedef struct {
  unsigned length;
  unsigned char bytes[32];
} RpkLeaf;
typedef struct {
  uint32_t domain;
  unsigned count;
  RpkLeaf leaves[16];
} RpkTree;
typedef struct {
  uint32_t domain;
  unsigned count;
  unsigned char root[32];
} RpkCommitment;
typedef struct {
  uint32_t domain;
  unsigned count, index, depth;
  unsigned char siblings[4][32];
} RpkProof;
typedef struct {
  unsigned hashes, requests, request_bytes, response_bytes, leaves_received;
} RpkCost;
int rpk_commit(const RpkTree *tree, RpkCommitment *out);
int rpk_prove(const RpkTree *tree, unsigned index, RpkProof *out);
int rpk_verify(const RpkLeaf *leaf, const RpkProof *proof,
               const RpkCommitment *trusted);
/* Callback borrows request/reply only during this synchronous finite call.
   Zero success writes exact reply length; nonzero means unavailable response.
   Neither callbacks nor concurrent actors may mutate target/control objects. */
typedef int (*RpkExchange)(void *context, const unsigned char *request,
                           size_t request_length, unsigned char *reply,
                           size_t capacity, size_t *reply_length);
int rpk_serve(const RpkTree *tree, const unsigned char *request,
              size_t request_length, unsigned chunk, unsigned char *reply,
              size_t capacity, size_t *reply_length);
/* chunk1..108 changes stream segmentation; force_full0 skips proven-equal
   ranges,1 fetches every node,2 fetches each leaf and verifies only at the
   final staged-root check. All use the same final-root publication contract.
   All target/cost publication is atomic on success; no persistent store here.
 */
int rpk_repair(RpkTree *target, const RpkCommitment *trusted, unsigned chunk,
               unsigned force_full, RpkExchange exchange, void *context,
               RpkCost *cost);
#endif
