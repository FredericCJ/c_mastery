#include "owning_ring.h"
int mpo_erased_init(MpoErasedRing*r,void *const slots[4],void*s,MpoErasedOps ops){(void)r;(void)slots;(void)s;(void)ops;return MPO_INVALID;}
int mpo_erased_push(MpoErasedRing*r,const void*in){(void)r;(void)in;return MPO_INVALID;}
int mpo_erased_pop(MpoErasedRing*r,void*out){(void)r;(void)out;return MPO_INVALID;}
void mpo_erased_clear(MpoErasedRing*r){(void)r;}
