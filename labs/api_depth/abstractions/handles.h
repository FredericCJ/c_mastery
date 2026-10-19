#ifndef CMASTERY_ABSTRACTION_HANDLES_H
#define CMASTERY_ABSTRACTION_HANDLES_H
#include <stdatomic.h>
typedef enum { AH_OK,AH_INVALID,AH_STALE,AH_FULL,AH_BUSY,AH_DEAD,AH_NOMEM,AH_UNIMPLEMENTED } AhResult;
typedef struct { unsigned generation,occupied,retired,value; } AhSlot;
typedef struct { AhSlot slots[3];unsigned epoch,borrowers; } AhPool;
typedef struct { unsigned index,generation,epoch; } AhHandle;
void ah_pool_init(AhPool *);
AhResult ah_allocate(AhPool *,unsigned value,AhHandle *);
AhResult ah_read(const AhPool *,AhHandle,unsigned *);
AhResult ah_release(AhPool *,AhHandle);
AhResult ah_enter(AhPool *);
AhResult ah_leave(AhPool *);
AhResult ah_reuse(AhPool *);
typedef struct { atomic_uint controls,payloads,control_frees,payload_frees; } AhStats;
typedef struct AhShared AhShared;
AhResult ah_shared_create(unsigned,AhStats *,AhShared **);
AhResult ah_retain(AhShared *);
AhResult ah_drop(AhShared *);
AhResult ah_weak_retain(AhShared *);
AhResult ah_weak_drop(AhShared *);
AhResult ah_promote(AhShared *);
AhResult ah_counts(AhShared *,unsigned *,unsigned *);
AhResult ah_add(AhShared *,unsigned);
/* Strong ownership is required; raw access needs an independent access protocol. */
unsigned *ah_payload(AhShared *);
#endif
