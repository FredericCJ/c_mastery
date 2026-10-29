#include "merkle_repair.h"

int rpk_commit(const struct rpk_tree *tree, struct rpk_commitment *out)
{
    (void)tree;
    (void)out;
    return RPK_UNIMPLEMENTED;
}

int rpk_prove(const struct rpk_tree *tree, unsigned index,
              struct rpk_proof *out)
{
    (void)tree;
    (void)index;
    (void)out;
    return RPK_UNIMPLEMENTED;
}

int rpk_verify(const struct rpk_leaf *leaf, const struct rpk_proof *proof,
               const struct rpk_commitment *trusted)
{
    (void)leaf;
    (void)proof;
    (void)trusted;
    return RPK_UNIMPLEMENTED;
}

int rpk_serve(const struct rpk_tree *tree, const unsigned char *request,
              size_t request_length, unsigned chunk, unsigned char *reply,
              size_t capacity, size_t *reply_length)
{
    (void)tree;
    (void)request;
    (void)request_length;
    (void)chunk;
    (void)reply;
    (void)capacity;
    (void)reply_length;
    return RPK_UNIMPLEMENTED;
}

int rpk_repair(struct rpk_tree *target, const struct rpk_commitment *trusted,
               unsigned chunk, unsigned force_full, rpk_exchange_fn exchange,
               void *context, struct rpk_cost *cost)
{
    (void)target;
    (void)trusted;
    (void)chunk;
    (void)force_full;
    (void)exchange;
    (void)context;
    (void)cost;
    return RPK_UNIMPLEMENTED;
}
