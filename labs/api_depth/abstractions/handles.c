#include "handles.h"
/* Intentionally unfinished learner mechanisms. Private implementations are excluded. */
void ah_pool_init(AhPool *p){(void)p;}
AhResult ah_allocate(AhPool *p,unsigned v,AhHandle *h){(void)p;(void)v;(void)h;return AH_UNIMPLEMENTED;}
AhResult ah_read(const AhPool *p,AhHandle h,unsigned *v){(void)p;(void)h;(void)v;return AH_UNIMPLEMENTED;}
AhResult ah_release(AhPool *p,AhHandle h){(void)p;(void)h;return AH_UNIMPLEMENTED;}
AhResult ah_enter(AhPool *p){(void)p;return AH_UNIMPLEMENTED;}
AhResult ah_leave(AhPool *p){(void)p;return AH_UNIMPLEMENTED;}
AhResult ah_reuse(AhPool *p){(void)p;return AH_UNIMPLEMENTED;}
AhResult ah_shared_create(unsigned v,AhStats *s,AhShared **o){(void)v;(void)s;(void)o;return AH_UNIMPLEMENTED;}
AhResult ah_retain(AhShared *s){(void)s;return AH_UNIMPLEMENTED;}
AhResult ah_drop(AhShared *s){(void)s;return AH_UNIMPLEMENTED;}
AhResult ah_weak_retain(AhShared *s){(void)s;return AH_UNIMPLEMENTED;}
AhResult ah_weak_drop(AhShared *s){(void)s;return AH_UNIMPLEMENTED;}
AhResult ah_promote(AhShared *s){(void)s;return AH_UNIMPLEMENTED;}
AhResult ah_counts(AhShared *s,unsigned *a,unsigned *b){(void)s;(void)a;(void)b;return AH_UNIMPLEMENTED;}
AhResult ah_add(AhShared *s,unsigned v){(void)s;(void)v;return AH_UNIMPLEMENTED;}
unsigned *ah_payload(AhShared *s){(void)s;return 0;}
