#include "law_traces.h"
int mpt_assign(MptOwner *a,MptView b,MptAllocator c){(void)a;(void)b;(void)c;return MPT_INVALID;}
void mpt_drop(MptOwner *a){(void)a;}
MptView mpt_view(const MptOwner *a){(void)a;return (MptView){0};}
int mpt_equal(MptView a,MptView b,unsigned c,unsigned *d){(void)a;(void)b;(void)c;(void)d;return MPT_INVALID;}
int mpt_hash(MptView a,unsigned b,uint64_t *c){(void)a;(void)b;(void)c;return MPT_INVALID;}
