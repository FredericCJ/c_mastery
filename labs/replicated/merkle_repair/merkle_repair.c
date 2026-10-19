#include "merkle_repair.h"
int rpk_commit(const RpkTree *tree, RpkCommitment *out) {
  (void)tree;
  (void)out;
  return RPK_UNIMPLEMENTED;
}
int rpk_prove(const RpkTree *tree, unsigned index, RpkProof *out) {
  (void)tree;
  (void)index;
  (void)out;
  return RPK_UNIMPLEMENTED;
}
int rpk_verify(const RpkLeaf *leaf, const RpkProof *proof,
               const RpkCommitment *trusted) {
  (void)leaf;
  (void)proof;
  (void)trusted;
  return RPK_UNIMPLEMENTED;
}
int rpk_serve(const RpkTree *tree, const unsigned char *request,
              size_t request_length, unsigned chunk, unsigned char *reply,
              size_t capacity, size_t *reply_length) {
  (void)tree;
  (void)request;
  (void)request_length;
  (void)chunk;
  (void)reply;
  (void)capacity;
  (void)reply_length;
  return RPK_UNIMPLEMENTED;
}
int rpk_repair(RpkTree *target, const RpkCommitment *trusted, unsigned chunk,
               unsigned force_full, RpkExchange exchange, void *context,
               RpkCost *cost) {
  (void)target;
  (void)trusted;
  (void)chunk;
  (void)force_full;
  (void)exchange;
  (void)context;
  (void)cost;
  return RPK_UNIMPLEMENTED;
}
