#ifndef CMASTERY_CONTRACT_OWNERSHIP_H
#define CMASTERY_CONTRACT_OWNERSHIP_H
#include "bounds.h"
enum { AC_RESOURCES=4, AC_EVENTS=64 };
typedef struct { unsigned resource,request,acquire; } AcEvent;
typedef struct {
  unsigned held[AC_RESOURCES], request[AC_RESOURCES];
  unsigned acquisitions[AC_RESOURCES], releases[AC_RESOURCES];
  unsigned consume_on_error[AC_RESOURCES];
  AcEvent events[AC_EVENTS];unsigned count;
} AcLedger;
AcStatus ac_resource_acquire(AcLedger *,unsigned resource,unsigned request,int fail_before,int lose_ack);
AcStatus ac_resource_query(const AcLedger *,unsigned resource,unsigned request,int *owned);
AcStatus ac_resource_close(AcLedger *,unsigned resource,int fail);
typedef struct { AcStatus first;unsigned retained; } AcCloseReport;
AcCloseReport ac_close_all(AcLedger *,unsigned failure_mask);
typedef enum { AC_NEW,AC_PARTIAL,AC_ACTIVE,AC_FAILED,AC_STOPPED,AC_DESTROYED } AcState;
typedef struct { AcState state;unsigned mask,epoch;int optional; } AcSession;
AcStatus ac_session_admit(const AcSession *);
AcStatus ac_session_init(AcSession *,AcLedger *,unsigned failure_point,int optional);
AcStatus ac_session_stop(AcSession *,AcLedger *);
AcStatus ac_session_destroy(AcSession *);
AcStatus ac_session_restart(AcSession *,AcLedger *,int optional);
typedef struct { unsigned slot,generation; } AcToken;
typedef struct { unsigned live,generation;int value; } AcIdentity;
typedef struct { AcIdentity slots[2]; } AcTable;
AcStatus ac_table_alloc(AcTable *,int value,AcToken *);
AcStatus ac_table_free(AcTable *,AcToken);
AcStatus ac_table_get(const AcTable *,AcToken,int *);
typedef struct AcShared AcShared;
struct AcShared { unsigned strong,alive,releases;AcShared *children[2]; };
AcStatus ac_shared_retain(AcShared *);
AcStatus ac_shared_link(AcShared *,AcShared *);
AcStatus ac_shared_drop(AcShared *);
typedef struct { unsigned live,issued;int value; } AcLeaseOwner;
AcStatus ac_lease_read(const AcLeaseOwner *,unsigned token_issued,unsigned now,int *);
AcStatus ac_lease_reuse(AcLeaseOwner *,unsigned now,int value);
typedef struct { FBuffer bytes;uint32_t values;unsigned live; } AcEpochOwner;
typedef struct { FView bytes;uint32_t values; } AcSnapshot;
AcStatus ac_epoch_update(AcEpochOwner *,size_t index,uint8_t value);
AcStatus ac_epoch_snapshot(const AcEpochOwner *,AcSnapshot *);
AcStatus ac_epoch_read(const AcEpochOwner *,AcSnapshot,size_t index,uint8_t *);
AcStatus ac_epoch_reset(AcEpochOwner *);
#endif
