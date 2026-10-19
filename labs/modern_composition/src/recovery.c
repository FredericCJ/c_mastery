#include "recovery.h"
int mc_recovery_init(McRecovery *s,McResources r) {(void)s;(void)r;return 0;}
McRecovered mc_publish_recoverable(McRecovery *s,const unsigned char *p,size_t n,V4Allocator a) {
    (void)s;(void)p;(void)n;(void)a; McRecovered r={0}; return r;
}
int mc_recover(McRecovery *s,uint32_t t,unsigned *e) {(void)s;(void)t;(void)e;return 0;}
int mc_retire_current(McRecovery *s,uint32_t *t) {(void)s;(void)t;return 0;}
