#ifndef TX_H
#define TX_H
#include <stddef.h>
enum { TX_OK, TX_ACQUIRE, TX_ZERO, TX_TIMEOUT, TX_INVALID, TX_CANCEL, TX_BUSY };
struct tx_owner { unsigned ids[4]; size_t count; };
struct tx_result { unsigned status; size_t committed; };
struct cv_tx {
 void *context;
 int (*acquire)(void *,unsigned);
 void (*release)(void *,unsigned);
 long (*write)(void *,const unsigned char *,size_t);
 int (*cancelled)(void *);
 unsigned busy;
};
/* Context and live data/owner spans are disjoint. Calls externally serialized.
   Callback must not mutate Tx, owner, input or unrelated objects. Reentry rejects.
   write: -1 timeout, 0 no progress, 1..remaining committed; other values invalid.
   Failure preserves owner; committed prefix cannot be rolled back. n<=16.
   fourth=1 inserts identity15 between10 and20. Successful owner is destroyed once. */
struct tx_result tx_run(struct cv_tx *,unsigned fourth,const unsigned char *,size_t,struct tx_owner *);
void tx_destroy(struct cv_tx *,struct tx_owner *);
#endif
