#include "transaction.h"
TxResult tx_run(Tx *t,unsigned fourth,const unsigned char *p,size_t n,TxOwner *out){
 (void)t;(void)fourth;(void)p;(void)n;(void)out;
 /* TODO acquire, validate progress, preserve ownership, reverse cleanup. */
 return (TxResult){TX_INVALID,0};
}
void tx_destroy(Tx *t,TxOwner *out){(void)t;(void)out;/* TODO */}
