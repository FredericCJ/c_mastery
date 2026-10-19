#ifndef CMASTERY_ABSTRACTION_SESSION_H
#define CMASTERY_ABSTRACTION_SESSION_H
#include "ownership.h"
#include "session_v1.h"
typedef enum { AS_OK,AS_INVALID,AS_FULL,AS_UNSUPPORTED,AS_FAILED,AS_UNIMPLEMENTED } AsStatus;
typedef enum { AS_NEW,AS_ACTIVE,AS_SUSPENDED,AS_STOPPED } AsState;
typedef enum { AS_START,AS_INCREMENT,AS_STOP,AS_SUSPEND,AS_RESUME,AS_CANCEL } AsEvent;
typedef struct { AsState state;unsigned count,ready;AcLedger *ledger; } AsSession;
typedef void (*AsConstructionObserver)(void *,AsSession *);
void as_session_init(AsSession *,AcLedger *);
AsStatus as_construct(AsSession *,unsigned fail_second,AsConstructionObserver,void *);
AsStatus as_event(AsSession *,AsEvent);
typedef struct AsOpaque AsOpaque;
AsStatus as_opaque_create(AcAllocate,AcLedger *,AsOpaque **);
AsStatus as_opaque_event(AsOpaque *,AsEvent);
AsStatus as_opaque_read(const AsOpaque *,unsigned *);
AsStatus as_opaque_destroy(AsOpaque *);
typedef struct { uint32_t version,flags;size_t bytes; } AsHeader;
typedef AsStatus (*AsRun)(void *,AsEvent);
typedef AsStatus (*AsFlush)(void *);
typedef struct { AsHeader header;AsRun run; } AsOpsV1;
typedef struct { AsOpsV1 prefix;AsFlush flush; } AsOpsV2;
typedef struct { AsRun run;AsFlush flush;void *context; } AsBackend;
typedef struct { AsSession *session;unsigned calls; } AsExtra;
extern const AsOpsV1 as_direct_ops;
extern const AsOpsV2 as_adapted_ops;
AsStatus as_backend(const void *descriptor,size_t available,void *context,AsBackend *);
AsStatus as_backend_run(AsBackend,AsEvent);
AsStatus as_backend_flush(AsBackend);
#endif
