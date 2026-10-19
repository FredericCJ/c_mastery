#include "checkpoint_async.h"
CPAStatus cpa_init(CPAQueue *q,unsigned n){(void)q;(void)n;return CPA_ERROR;}
CPAStatus cpa_offer(CPAQueue *q,const CPMessage *m,int b){(void)q;(void)m;(void)b;return CPA_ERROR;}
CPAStatus cpa_take(CPAQueue *q,CPAHandle *h,CPMessage *m,int b){(void)q;(void)h;(void)m;(void)b;return CPA_ERROR;}
CPAStatus cpa_finish(CPAQueue *q,CPAHandle h,int f){(void)q;(void)h;(void)f;return CPA_ERROR;}
CPAStatus cpa_close(CPAQueue *q,int a){(void)q;(void)a;return CPA_ERROR;}
CPAStatus cpa_wait_closed(CPAQueue *q,const struct timespec *t){(void)q;(void)t;return CPA_ERROR;}
CPAStatus cpa_destroy(CPAQueue *q){(void)q;return CPA_ERROR;}
