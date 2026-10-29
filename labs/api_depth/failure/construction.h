#ifndef CMASTERY_FAILURE_CONSTRUCTION_H
#define CMASTERY_FAILURE_CONSTRUCTION_H
#include "ownership.h"
#include "taxonomy.h"
enum ae_construction_state {
  AE_CNEW,
  AE_CPARTIAL,
  AE_CACTIVE,
  AE_CFAILED,
  AE_CSTOPPED
};
struct ae_transfer {
  unsigned resource, generation;
};
struct ae_construction {
  struct ac_ledger *ledger;
  struct ae_error error;
  unsigned mask, generation, count, order[4], published, committed;
  struct ae_transfer transfers[4];
  unsigned transfer_count, cleanup_style;
  enum ae_construction_state state;
};
struct ae_construction_handle {
  struct ae_construction *owner;
  unsigned generation;
};
enum ae_construction_style { AE_NESTED, AE_LABELS, AE_LOOP };
void ae_construction_init(struct ae_construction *, struct ac_ledger *);
enum ae_code ae_construct(struct ae_construction *, unsigned count, unsigned swapped,
                    unsigned fail_point, unsigned close_fail_mask,
                    enum ae_construction_style, struct ae_construction_handle *);
enum ae_code ae_cleanup(struct ae_construction *, unsigned close_fail_mask);
enum ae_code ae_construction_read(struct ae_construction_handle, unsigned *);
enum ae_code ae_transfer(struct ae_construction *, unsigned resource);
enum ae_code ae_after_publish(struct ae_construction *, unsigned fail_notification);
enum ae_code ae_compensate(struct ae_construction *, unsigned effect_still_present);
#endif
