#include "depth.h"
int b3_pump(struct b3_writer *w,b3_write_fn f,void *c,unsigned b){(void)w;(void)f;(void)c;(void)b;return B3_FAILED;}
int b3_start(struct b3_operation *o){(void)o;return B3_FAILED;}
int b3_event(struct b3_operation *o,unsigned g,enum b3_event e){(void)o;(void)g;(void)e;return B3_FAILED;}
int b3_ack(struct b3_device *d,unsigned m){(void)d;(void)m;return B3_FAILED;}
int b3_read_data(struct b3_device *d,unsigned *o){(void)d;(void)o;return B3_FAILED;}
int b3_due(uint8_t n,uint8_t s,uint8_t d,int *o){(void)n;(void)s;(void)d;(void)o;return B3_FAILED;}
void b3_mailbox_init(struct b3_mailbox *m){atomic_init(&m->ready,0);m->payload=0;}
int b3_publish(struct b3_mailbox *m,int v){(void)m;(void)v;return B3_FAILED;}
int b3_take(struct b3_mailbox *m,int *o){(void)m;(void)o;return B3_FAILED;}
memory_order b3_load_order(void){return memory_order_relaxed;}
memory_order b3_store_order(void){return memory_order_relaxed;}
