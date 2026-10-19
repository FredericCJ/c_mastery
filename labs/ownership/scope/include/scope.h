#ifndef CMASTERY_OWNERSHIP_SCOPE_DEPTH_H
#define CMASTERY_OWNERSHIP_SCOPE_DEPTH_H
#include "ownership.h"
#include "errors.h"
typedef struct { unsigned count; OwWhen policy[8]; unsigned fail_before; er_code raised; } OcsPlan;
typedef struct { er_code original,converted; unsigned registered,trace[8],seen_error[8],count; } OcsReport;
/* Ordinary callbacks are nonfailing. Errors are converted before final cleanup. */
OwStatus ocs_run(const OcsPlan *,unsigned *external_effects,OcsReport *);
typedef enum { OCS_RELEASED, OCS_RETAINED, OCS_RELEASED_ERROR } OcsClose;
typedef OcsClose (*OcsCloser)(void *,unsigned identity);
typedef struct { OcsCloser close;void *context;unsigned identity; } OcsEntry;
typedef struct OcsRecovery OcsRecovery;
typedef struct { OcsRecovery *owner;uint32_t generation; } OcsTicket;
struct OcsRecovery { OcsEntry entry[8];unsigned count,active,finishing,sealed,errors;uint32_t generation; };
typedef struct { unsigned pending,close_errors,recovery_required; } OcsRecovered;
OwStatus ocs_begin(OcsRecovery *,OcsTicket *);
OwStatus ocs_push(OcsTicket,OcsCloser,void *,unsigned);
OwStatus ocs_recover(OcsTicket,OcsRecovered *);
#endif
