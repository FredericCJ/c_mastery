#include "transaction.h"
int bf_construct(unsigned s,B2Backend b,unsigned*p){(void)s;(void)b;(void)p;return B2_FAILED;}
void bf_tx_init(BfTx*t){*t=(BfTx){0};t->stage=4;}
int bf_tx_begin(BfTx*t,BfTxBackend b,unsigned*p){(void)t;(void)b;(void)p;return B2_FAILED;}
int bf_tx_close(BfTx*t){(void)t;return B2_FAILED;}
