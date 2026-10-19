#include "sharing.h"
#include <errno.h>
int ts_sum(const int32_t *p,size_t n,unsigned w,int s,int64_t *o){(void)p;(void)n;(void)w;(void)s;(void)o;return ENOSYS;}
int ts_snapshot_sum(TSInput *p,unsigned w,TSSnapshotHook h,void *c,int64_t *o,unsigned *v){(void)p;(void)w;(void)h;(void)c;(void)o;(void)v;return ENOSYS;}
int ts_queue_init(TSQueue *q){(void)q;return ENOSYS;}
int ts_enqueue(TSQueue *q,unsigned id){(void)q;(void)id;return ENOSYS;}
int ts_dequeue(TSQueue *q,unsigned *o){(void)q;(void)o;return ENOSYS;}
int ts_settle(TSQueue *q,unsigned id,unsigned s){(void)q;(void)id;(void)s;return ENOSYS;}
int ts_queue_close(TSQueue *q){(void)q;return ENOSYS;}
int ts_queue_inspect(TSQueue *q,unsigned i,unsigned *s,unsigned *t,unsigned *n){(void)q;(void)i;(void)s;(void)t;(void)n;return ENOSYS;}
int ts_queue_destroy(TSQueue *q){(void)q;return ENOSYS;}
int ts_histogram(const uint32_t *p,size_t n,size_t b,unsigned m,size_t cap,uint64_t *o,TSResources *r){(void)p;(void)n;(void)b;(void)m;(void)cap;(void)o;(void)r;return ENOSYS;}
