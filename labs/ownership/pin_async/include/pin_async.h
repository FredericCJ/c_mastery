#ifndef CMASTERY_OWNERSHIP_PIN_ASYNC_H
#define CMASTERY_OWNERSHIP_PIN_ASYNC_H
#include "ownership.h"
#include "foundation.h"
struct opa_machine {struct ow_pin pin;struct f_async async;uint64_t executor;unsigned notified,recovery;int32_t result;};
/* Externally serialized core; address and logical identities are not authority
 * proofs. The hosted adapter checks actual pthread identity and joins its worker. */
enum ow_status opa_init(struct opa_machine *,const void *,uint64_t owner,uint64_t executor);
enum ow_status opa_submit(struct opa_machine *,const void *,uint64_t owner,uint32_t *ticket);
enum ow_status opa_cancel(struct opa_machine *,uint32_t,uint64_t owner);
enum ow_status opa_timeout(struct opa_machine *,uint32_t,uint64_t owner);
enum ow_status opa_notify(struct opa_machine *,uint32_t,uint64_t executor,int32_t);
/* Trusted backend quiescence is a prerequisite, not inferred from notified. */
enum ow_status opa_finish(struct opa_machine *,uint32_t,uint64_t owner,int32_t *);
enum ow_status opa_rebind(struct opa_machine *,const void *,uint64_t owner);
#endif
