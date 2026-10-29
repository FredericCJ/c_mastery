#ifndef BF_TRANSACTION_H
#define BF_TRANSACTION_H
#include "depth.h"
/* style0 retained provider,1 early return,2 nested,3 cleanup label. */
int bf_construct(unsigned,struct b2_backend,unsigned *);
struct bf_tx_backend {int (*acquire)(void *,unsigned);int (*release)(void *,unsigned);int (*commit)(void *);void *context;};
enum { BF_TX_IDLE, BF_TX_LIVE, BF_TX_PENDING, BF_TX_DONE };
enum { BF_TX_RECOVERY=7 };
struct bf_tx {struct bf_tx_backend backend;unsigned state,mask,stage,failed_cleanup;int primary,cleanup_error;};
void bf_tx_init(struct bf_tx *);
int bf_tx_begin(struct bf_tx *,struct bf_tx_backend,unsigned *);
int bf_tx_close(struct bf_tx *);
#endif
