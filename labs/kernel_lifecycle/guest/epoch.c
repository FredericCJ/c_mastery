#include "epoch.h"
int ke_enter(KEpoch*s,unsigned i){(void)s;(void)i;return 0;}
int ke_leave(KEpoch*s,unsigned i){(void)s;(void)i;return 0;}
int ke_replace(KEpoch*s){(void)s;return 0;}
int ke_reclaim(KEpoch*s){(void)s;return 0;}
int ke_close(KEpoch*s){(void)s;return 0;}
int ke_requeue(const KEpoch*s,uint32_t g){(void)s;(void)g;return 0;}
