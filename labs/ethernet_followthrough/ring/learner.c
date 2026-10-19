#include "ring.h"
/* Fill every contract from permitted primitives; this file compiles but must
 * fail semantic acceptance. Implement the target-independent ownership core. */
int en_range(uint32_t a,size_t b){(void)a;(void)b;return ED_INVALID;}
int en_init(EnRing *a,EnMemory *b,uint32_t c,const EnBus *d,EpPhy *e,const unsigned char f[6]){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return ED_INVALID;}
int en_reset_begin(EnRing *a,uint32_t b,unsigned c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return ED_INVALID;}
int en_reset_step(EnRing *a,uint32_t b){(void)a;(void)b;return ED_INVALID;}
int en_start(EnRing *a){(void)a;return ED_INVALID;}
int en_tx_copy(EnRing *a,const void *b,size_t c,EnTicket *d){(void)a;(void)b;(void)c;(void)d;return ED_INVALID;}
int en_tx_publish(EnRing *a,EnTicket b){(void)a;(void)b;return ED_INVALID;}
int en_tx_reap(EnRing *a,EnTicket b,uint32_t *c){(void)a;(void)b;(void)c;return ED_INVALID;}
int en_rx_take(EnRing *a,EnFrame *b){(void)a;(void)b;return ED_INVALID;}
int en_rx_release(EnRing *a,EnTicket b){(void)a;(void)b;return ED_INVALID;}
int en_status(EnRing *a,uint32_t *b){(void)a;(void)b;return ED_INVALID;}
int en_stop_begin(EnRing *a,uint32_t b,unsigned c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return ED_INVALID;}
int en_stop_step(EnRing *a,uint32_t b){(void)a;(void)b;return ED_INVALID;}
