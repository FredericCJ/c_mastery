#include "owning_ring.h"
int mpo_manual_init(struct mpo_blob_ring*r,struct mpo_blob_ops ops){(void)r;(void)ops;return MPO_INVALID;}
int mpo_manual_push(struct mpo_blob_ring*r,const struct mpo_blob*in){(void)r;(void)in;return MPO_INVALID;}
int mpo_manual_pop(struct mpo_blob_ring*r,struct mpo_blob*out){(void)r;(void)out;return MPO_INVALID;}
void mpo_manual_clear(struct mpo_blob_ring*r){(void)r;}
