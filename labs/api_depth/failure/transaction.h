#ifndef CMASTERY_FAILURE_TRANSACTION_H
#define CMASTERY_FAILURE_TRANSACTION_H
#include "public_error.h"
typedef struct {
  unsigned values[3], count, version;
} AeConfig;
typedef enum { AE_STRONG, AE_BASIC, AE_PARTIAL } AeGuarantee;
typedef struct {
  AeCode code;
  unsigned committed, accepted;
} AeUpdate;
typedef void (*AeObserve)(void *, const AeConfig *);
/* fail=0 none;1 validation boundary;2 staging;3..n+2 before copy;
 * n+3 before commit. Valid inputs <=9; capacity,n<=3. */
AeUpdate ae_replace(AeConfig *, const unsigned *, unsigned n, unsigned capacity,
                    AeGuarantee, unsigned fail, unsigned notify_failed,
                    AeObserve, void *);
typedef struct {
  unsigned id, delta;
} AeDone;
typedef struct {
  AeDone done[2];
  unsigned count, next, pending_id, pending_delta, effects, value;
} AeJournal;
/* crash=1 after intent/before effect;2 after effect/before done;
 * 3 after done/before acknowledgment;0 uninterrupted. */
AeCode ae_journal(AeJournal *, unsigned id, unsigned delta, unsigned crash);
#endif
