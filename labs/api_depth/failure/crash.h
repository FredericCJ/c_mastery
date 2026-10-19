#ifndef CMASTERY_FAILURE_CRASH_H
#define CMASTERY_FAILURE_CRASH_H
#include "construction.h"
#include "transaction.h"
typedef struct {
  unsigned build, address, state;
} AeFault;
AeCode ae_fault_encode(AeFault, unsigned char *, size_t);
AeCode ae_fault_decode(const unsigned char *, size_t, AeFault *);
typedef struct {
  unsigned char bytes[2][8];
} AeCrashSlots;
AeCode ae_fault_select(const AeCrashSlots *, AeFault *, unsigned *generation);
/* cut1..9 reset before byte write,10 after commit;0 no reset. */
AeCode ae_fault_save(AeCrashSlots *, AeFault, unsigned cut);
typedef struct {
  AcLedger ledger;
  AeConstruction owner;
  AeJournal effects;
  unsigned attempts, done;
} AeRecovery;
void ae_recovery_init(AeRecovery *);
/* cut1 beforecleanup,2 aftercleanup,3 afterconstruction,4 aftereffect,
 *5 afterdone;0 uninterrupted. Persistent-state model, not hardware reset. */
AeCode ae_recover(AeRecovery *, const unsigned char *, size_t,
                  unsigned expected_build, unsigned cut);
#endif
