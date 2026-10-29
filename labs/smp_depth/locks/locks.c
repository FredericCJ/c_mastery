#include "locks.h"
int sl_init(struct sl_lock *l,unsigned k,unsigned r,unsigned s){(void)l;(void)k;(void)r;(void)s;return SL_INVALID;}
void sl_node_init(struct sl_node *n){(void)n;}
int sl_ticket_reserve(struct sl_lock *l,struct sl_guard *g){(void)l;(void)g;return SL_INVALID;}
int sl_ticket_ready(struct sl_guard *g,struct sl_stats *s){(void)g;(void)s;return SL_INVALID;}
int sl_enter(struct sl_lock *l,struct sl_node *n,struct sl_guard *g,struct sl_stats *s){(void)l;(void)n;(void)g;(void)s;return SL_INVALID;}
int sl_leave(struct sl_guard *g,struct sl_stats *s){(void)g;(void)s;return SL_INVALID;}
int sl_irq_enter(struct sl_context *c,struct sl_port p,struct sl_lock *l,struct sl_node *n,struct sl_irq_guard *g,struct sl_stats *s){(void)c;(void)p;(void)l;(void)n;(void)g;(void)s;return SL_INVALID;}
int sl_irq_leave(struct sl_context *c,struct sl_port p,struct sl_irq_guard *g,struct sl_stats *s){(void)c;(void)p;(void)g;(void)s;return SL_INVALID;}
