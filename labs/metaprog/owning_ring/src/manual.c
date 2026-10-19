#include "owning_ring.h"
int mpo_manual_init(MpoBlobRing*r,MpoBlobOps ops){(void)r;(void)ops;return MPO_INVALID;}
int mpo_manual_push(MpoBlobRing*r,const MpoBlob*in){(void)r;(void)in;return MPO_INVALID;}
int mpo_manual_pop(MpoBlobRing*r,MpoBlob*out){(void)r;(void)out;return MPO_INVALID;}
void mpo_manual_clear(MpoBlobRing*r){(void)r;}
