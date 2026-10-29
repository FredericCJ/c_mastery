#include "owning_ring.h"
int mpo_erased_init(struct mpo_erased_ring*r,void *const slots[4],void*s,struct mpo_erased_ops ops){(void)r;(void)slots;(void)s;(void)ops;return MPO_INVALID;}
int mpo_erased_push(struct mpo_erased_ring*r,const void*in){(void)r;(void)in;return MPO_INVALID;}
int mpo_erased_pop(struct mpo_erased_ring*r,void*out){(void)r;(void)out;return MPO_INVALID;}
void mpo_erased_clear(struct mpo_erased_ring*r){(void)r;}
