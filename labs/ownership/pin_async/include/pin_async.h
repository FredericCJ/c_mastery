#ifndef CMASTERY_OWNERSHIP_PIN_ASYNC_H
#define CMASTERY_OWNERSHIP_PIN_ASYNC_H
#include "ownership.h"
#include "foundation.h"
typedef struct {OwPin pin;FAsync async;uint64_t executor;unsigned notified,recovery;int32_t result;} OpaMachine;
/* Externally serialized core; address and logical identities are not authority
 * proofs. The hosted adapter checks actual pthread identity and joins its worker. */
OwStatus opa_init(OpaMachine *,const void *,uint64_t owner,uint64_t executor);
OwStatus opa_submit(OpaMachine *,const void *,uint64_t owner,uint32_t *ticket);
OwStatus opa_cancel(OpaMachine *,uint32_t,uint64_t owner);
OwStatus opa_timeout(OpaMachine *,uint32_t,uint64_t owner);
OwStatus opa_notify(OpaMachine *,uint32_t,uint64_t executor,int32_t);
/* Trusted backend quiescence is a prerequisite, not inferred from notified. */
OwStatus opa_finish(OpaMachine *,uint32_t,uint64_t owner,int32_t *);
OwStatus opa_rebind(OpaMachine *,const void *,uint64_t owner);
#endif
