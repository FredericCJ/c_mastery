#ifndef CMASTERY_REPLICATED_MERKLE_REPAIR_H
#define CMASTERY_REPLICATED_MERKLE_REPAIR_H
#include <stddef.h>
#include <stdint.h>

enum
{
    RPK_OK,
    RPK_INVALID,
    RPK_FULL,
    RPK_PROOF,
    RPK_PROTOCOL,
    RPK_MISSING,
    RPK_UNIMPLEMENTED
};

struct rpk_leaf
{
    unsigned length;
    unsigned char bytes[32];
};

struct rpk_tree
{
    uint32_t domain;
    unsigned count;
    struct rpk_leaf leaves[16];
};

struct rpk_commitment
{
    uint32_t domain;
    unsigned count;
    unsigned char root[32];
};

struct rpk_proof
{
    uint32_t domain;
    unsigned count, index, depth;
    unsigned char siblings[4][32];
};

struct rpk_cost
{
    unsigned hashes, requests, request_bytes, response_bytes, leaves_received;
};

int rpk_commit(const struct rpk_tree *tree, struct rpk_commitment *out);
int rpk_prove(const struct rpk_tree *tree, unsigned index,
              struct rpk_proof *out);
int rpk_verify(const struct rpk_leaf *leaf, const struct rpk_proof *proof,
               const struct rpk_commitment *trusted);
/* Callback borrows request/reply only during this synchronous finite call.
   Zero success writes exact reply length; nonzero means unavailable response.
   Neither callbacks nor concurrent actors may mutate target/control objects. */
typedef int (*rpk_exchange_fn)(void *context, const unsigned char *request,
                               size_t request_length, unsigned char *reply,
                               size_t capacity, size_t *reply_length);
int rpk_serve(const struct rpk_tree *tree, const unsigned char *request,
              size_t request_length, unsigned chunk, unsigned char *reply,
              size_t capacity, size_t *reply_length);
/* chunk1..108 changes stream segmentation; force_full0 skips proven-equal
   ranges,1 fetches every node,2 fetches each leaf and verifies only at the
   final staged-root check. All use the same final-root publication contract.
   All target/cost publication is atomic on success; no persistent store here.
 */
int rpk_repair(struct rpk_tree *target, const struct rpk_commitment *trusted,
               unsigned chunk, unsigned force_full, rpk_exchange_fn exchange,
               void *context, struct rpk_cost *cost);
#endif
