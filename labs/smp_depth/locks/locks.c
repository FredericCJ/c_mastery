#include "locks.h"
int sl_init(SlLock *l,unsigned k,unsigned r,unsigned s){(void)l;(void)k;(void)r;(void)s;return SL_INVALID;}
void sl_node_init(SlNode *n){(void)n;}
int sl_ticket_reserve(SlLock *l,SlGuard *g){(void)l;(void)g;return SL_INVALID;}
int sl_ticket_ready(SlGuard *g,SlStats *s){(void)g;(void)s;return SL_INVALID;}
int sl_enter(SlLock *l,SlNode *n,SlGuard *g,SlStats *s){(void)l;(void)n;(void)g;(void)s;return SL_INVALID;}
int sl_leave(SlGuard *g,SlStats *s){(void)g;(void)s;return SL_INVALID;}
int sl_irq_enter(SlContext *c,SlPort p,SlLock *l,SlNode *n,SlIrqGuard *g,SlStats *s){(void)c;(void)p;(void)l;(void)n;(void)g;(void)s;return SL_INVALID;}
int sl_irq_leave(SlContext *c,SlPort p,SlIrqGuard *g,SlStats *s){(void)c;(void)p;(void)g;(void)s;return SL_INVALID;}
