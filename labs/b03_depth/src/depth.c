#include "depth.h"
int b3_pump(B3Writer *w,B3Write f,void *c,unsigned b){(void)w;(void)f;(void)c;(void)b;return B3_FAILED;}
int b3_start(B3Operation *o){(void)o;return B3_FAILED;}
int b3_event(B3Operation *o,unsigned g,enum B3Event e){(void)o;(void)g;(void)e;return B3_FAILED;}
int b3_ack(B3Device *d,unsigned m){(void)d;(void)m;return B3_FAILED;}
int b3_read_data(B3Device *d,unsigned *o){(void)d;(void)o;return B3_FAILED;}
int b3_due(uint8_t n,uint8_t s,uint8_t d,int *o){(void)n;(void)s;(void)d;(void)o;return B3_FAILED;}
void b3_mailbox_init(B3Mailbox *m){atomic_init(&m->ready,0);m->payload=0;}
int b3_publish(B3Mailbox *m,int v){(void)m;(void)v;return B3_FAILED;}
int b3_take(B3Mailbox *m,int *o){(void)m;(void)o;return B3_FAILED;}
memory_order b3_load_order(void){return memory_order_relaxed;}
memory_order b3_store_order(void){return memory_order_relaxed;}
