#ifndef CMASTERY_FAILURE_CRASH_H
#define CMASTERY_FAILURE_CRASH_H
#include "construction.h"
#include "transaction.h"

struct ae_fault
{
    unsigned build, address, state;
};
enum ae_code ae_fault_encode(struct ae_fault, unsigned char *, size_t);
enum ae_code ae_fault_decode(const unsigned char *, size_t, struct ae_fault *);

struct ae_crash_slots
{
    unsigned char bytes[2][8];
};
enum ae_code ae_fault_select(const struct ae_crash_slots *, struct ae_fault *,
                             unsigned *generation);
/* cut1..9 reset before byte write,10 after commit;0 no reset. */
enum ae_code ae_fault_save(struct ae_crash_slots *, struct ae_fault,
                           unsigned cut);

struct ae_recovery
{
    struct ac_ledger ledger;
    struct ae_construction owner;
    struct ae_journal effects;
    unsigned attempts, done;
};

void ae_recovery_init(struct ae_recovery *);
/* cut1 beforecleanup,2 aftercleanup,3 afterconstruction,4 aftereffect,
 *5 afterdone;0 uninterrupted. Persistent-state model, not hardware reset. */
enum ae_code ae_recover(struct ae_recovery *, const unsigned char *, size_t,
                        unsigned expected_build, unsigned cut);
#endif
