#ifndef BF_TRANSACTION_H
#define BF_TRANSACTION_H
#include "depth.h"
/* style0 retained provider,1 early return,2 nested,3 cleanup label. */
int bf_construct(unsigned,B2Backend,unsigned *);
typedef struct {int (*acquire)(void *,unsigned);int (*release)(void *,unsigned);int (*commit)(void *);void *context;} BfTxBackend;
enum { BF_TX_IDLE, BF_TX_LIVE, BF_TX_PENDING, BF_TX_DONE };
enum { BF_TX_RECOVERY=7 };
typedef struct {BfTxBackend backend;unsigned state,mask,stage,failed_cleanup;int primary,cleanup_error;} BfTx;
void bf_tx_init(BfTx *);
int bf_tx_begin(BfTx *,BfTxBackend,unsigned *);
int bf_tx_close(BfTx *);
#endif
