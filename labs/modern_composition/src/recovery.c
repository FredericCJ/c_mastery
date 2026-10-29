#include "recovery.h"
int mc_recovery_init(struct mc_recovery *s,struct mc_resources r) {(void)s;(void)r;return 0;}
struct mc_recovered mc_publish_recoverable(struct mc_recovery *s,const unsigned char *p,size_t n,struct v4_allocator a) {
    (void)s;(void)p;(void)n;(void)a; struct mc_recovered r={0}; return r;
}
int mc_recover(struct mc_recovery *s,uint32_t t,unsigned *e) {(void)s;(void)t;(void)e;return 0;}
int mc_retire_current(struct mc_recovery *s,uint32_t *t) {(void)s;(void)t;return 0;}
