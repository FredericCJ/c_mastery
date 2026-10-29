#include "transaction.h"
struct tx_result tx_run(struct cv_tx *t,unsigned fourth,const unsigned char *p,size_t n,struct tx_owner *out){
 (void)t;(void)fourth;(void)p;(void)n;(void)out;
 /* TODO acquire, validate progress, preserve ownership, reverse cleanup. */
 return (struct tx_result){TX_INVALID,0};
}
void tx_destroy(struct cv_tx *t,struct tx_owner *out){(void)t;(void)out;/* TODO */}
