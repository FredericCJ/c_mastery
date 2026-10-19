#include "lifecycle.h"
#include <errno.h>
TLPool *tl_pool_start(unsigned n,unsigned f,int a,int *e){(void)n;(void)f;(void)a;if(e)*e=ENOSYS;return 0;}
int tl_pool_collect(TLPool *p,TLJoin j,void *c,TLStats *o){(void)p;(void)j;(void)c;(void)o;return ENOSYS;}
int tl_pool_dispose(TLPool **p){(void)p;return ENOSYS;}
int tl_monitor_init(TLMonitor *m){(void)m;return ENOSYS;}
int tl_monitor_destroy(TLMonitor *m){(void)m;return ENOSYS;}
void *tl_detached_creator(void *a){(void)a;return 0;}
