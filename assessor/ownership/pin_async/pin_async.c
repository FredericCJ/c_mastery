#include "pin_async.h"
#ifndef OPA_MUTATION
#define OPA_MUTATION 0
#endif
static OwStatus admitted(const OpaMachine *m,uint32_t ticket,uint64_t owner){if(!m||owner!=m->pin.thread)return OW_INVALID;if(ticket!=m->async.generation)return OW_STALE;if(!m->pin.pending)return OW_INVALID;return OW_OK;}
OwStatus opa_init(OpaMachine *m,const void *address,uint64_t owner,uint64_t executor){if(!m||!address)return OW_INVALID;OpaMachine stage={0};OwStatus q=ow_pin_init(&stage.pin,address,owner);if(q!=OW_OK)return q;stage.executor=executor;*m=stage;return OW_OK;}
OwStatus opa_submit(OpaMachine *m,const void *address,uint64_t owner,uint32_t *out){
 if(!m||!out)return OW_INVALID;
 OwStatus q=ow_pin_check(&m->pin,address,owner);if(q!=OW_OK)return q;if(m->pin.pending)return OW_BUSY;
 if(m->async.generation==UINT32_MAX||(m->async.releases==UINT32_MAX&&OPA_MUTATION!=4))return OW_FULL;
 OpaMachine stage=*m;uint32_t ticket=0;if(f_submit(&stage.async,&ticket)!=F_OK)return OW_INVALID;q=ow_pin_submit(&stage.pin,address,owner);if(q!=OW_OK)return q;stage.notified=0;stage.recovery=0;stage.result=0;*m=stage;*out=ticket;return OW_OK;
}
OwStatus opa_cancel(OpaMachine *m,uint32_t ticket,uint64_t owner){OwStatus q=admitted(m,ticket,owner);if(q!=OW_OK)return q;if(m->async.state==F_CANCEL_REQUESTED)return OW_OK;OpaMachine stage=*m;if(f_cancel(&stage.async,ticket)!=F_PENDING)return OW_INVALID;q=ow_pin_cancel(&stage.pin,owner);if(q!=OW_OK)return q;*m=stage;return OW_OK;}
OwStatus opa_timeout(OpaMachine *m,uint32_t ticket,uint64_t owner){OwStatus q=opa_cancel(m,ticket,owner);if(q==OW_OK){m->recovery=1;if(OPA_MUTATION==1)m->pin.pending=0;}return q;}
OwStatus opa_notify(OpaMachine *m,uint32_t ticket,uint64_t executor,int32_t result){if(!m||(executor!=m->executor&&OPA_MUTATION!=3))return OW_INVALID;if(ticket!=m->async.generation)return OW_STALE;if(!m->pin.pending||m->notified)return OW_INVALID;m->result=result;m->notified=1;if(OPA_MUTATION==2)m->pin.pending=0;return OW_OK;}
OwStatus opa_finish(OpaMachine *m,uint32_t ticket,uint64_t owner,int32_t *out){if(!out)return OW_INVALID;OwStatus q=admitted(m,ticket,owner);if(q!=OW_OK)return q;if(!m->notified)return OW_BUSY;OpaMachine stage=*m;if(f_complete(&stage.async,ticket)!=F_OK)return OW_FULL;q=ow_pin_complete(&stage.pin,owner);if(q!=OW_OK)return q;stage.recovery=0;*m=stage;*out=stage.result;return OW_OK;}
OwStatus opa_rebind(OpaMachine *m,const void *address,uint64_t owner){if(!m)return OW_INVALID;return ow_pin_rebind(&m->pin,address,owner);}
