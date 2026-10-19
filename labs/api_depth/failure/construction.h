#ifndef CMASTERY_FAILURE_CONSTRUCTION_H
#define CMASTERY_FAILURE_CONSTRUCTION_H
#include "ownership.h"
#include "taxonomy.h"
typedef enum {
  AE_CNEW,
  AE_CPARTIAL,
  AE_CACTIVE,
  AE_CFAILED,
  AE_CSTOPPED
} AeConstructionState;
typedef struct {
  unsigned resource, generation;
} AeTransfer;
typedef struct {
  AcLedger *ledger;
  AeError error;
  unsigned mask, generation, count, order[4], published, committed;
  AeTransfer transfers[4];
  unsigned transfer_count, cleanup_style;
  AeConstructionState state;
} AeConstruction;
typedef struct {
  AeConstruction *owner;
  unsigned generation;
} AeConstructionHandle;
typedef enum { AE_NESTED, AE_LABELS, AE_LOOP } AeConstructionStyle;
void ae_construction_init(AeConstruction *, AcLedger *);
AeCode ae_construct(AeConstruction *, unsigned count, unsigned swapped,
                    unsigned fail_point, unsigned close_fail_mask,
                    AeConstructionStyle, AeConstructionHandle *);
AeCode ae_cleanup(AeConstruction *, unsigned close_fail_mask);
AeCode ae_construction_read(AeConstructionHandle, unsigned *);
AeCode ae_transfer(AeConstruction *, unsigned resource);
AeCode ae_after_publish(AeConstruction *, unsigned fail_notification);
AeCode ae_compensate(AeConstruction *, unsigned effect_still_present);
#endif
