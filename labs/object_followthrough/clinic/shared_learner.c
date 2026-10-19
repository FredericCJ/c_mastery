#include "shared.h"
int mc_shared_init(McShared *a,McAllocator b){(void)a;(void)b;return -1;}
int mc_shared_retain(McShared *a,McShared **b){(void)a;(void)b;return -1;}
int mc_shared_add(McShared *a,unsigned b){(void)a;(void)b;return -1;}
int mc_shared_read(McShared *a,unsigned *b){(void)a;(void)b;return -1;}
int mc_shared_drop(McShared **a){(void)a;return -1;}
int mc_shared_finish(McShared *a){(void)a;return -1;}
