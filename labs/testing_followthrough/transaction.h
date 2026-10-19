#ifndef TX_H
#define TX_H
#include <stddef.h>
enum { TX_OK, TX_ACQUIRE, TX_ZERO, TX_TIMEOUT, TX_INVALID, TX_CANCEL, TX_BUSY };
typedef struct { unsigned ids[4]; size_t count; } TxOwner;
typedef struct { unsigned status; size_t committed; } TxResult;
typedef struct {
 void *context;
 int (*acquire)(void *,unsigned);
 void (*release)(void *,unsigned);
 long (*write)(void *,const unsigned char *,size_t);
 int (*cancelled)(void *);
 unsigned busy;
} Tx;
/* Context and live data/owner spans are disjoint. Calls externally serialized.
   Callback must not mutate Tx, owner, input or unrelated objects. Reentry rejects.
   write: -1 timeout, 0 no progress, 1..remaining committed; other values invalid.
   Failure preserves owner; committed prefix cannot be rolled back. n<=16.
   fourth=1 inserts identity15 between10 and20. Successful owner is destroyed once. */
TxResult tx_run(Tx *,unsigned fourth,const unsigned char *,size_t,TxOwner *);
void tx_destroy(Tx *,TxOwner *);
#endif
