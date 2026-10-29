#ifndef SF_VM_SERVICE_H
#define SF_VM_SERVICE_H
#include "execution.h"
#define SFV_IMAGE 256u
#define SFV_RESULT 64u
enum {SFV_OK=0,SFV_RUNNING=100,SFV_ARGUMENT,SFV_BUSY,SFV_STALE,SFV_CONFLICT,
 SFV_CANCELLED,SFV_CAPACITY,SFV_FOREIGN,SFV_UNIMPLEMENTED};
struct sfv_request {uint64_t session,id;uint32_t fuel,gc,length,reserved;uint8_t image[SFV_IMAGE];};
struct sfv_handle {uint64_t session,id;uint32_t instance,reserved;struct cvm_value value;};
struct sfv_slot {struct vg_vs execution;struct sfv_request request;uint64_t session,high;
 unsigned state,status;struct cvm_value result;uint8_t bytes[SFV_RESULT];size_t length;};
struct sfv_service {struct sfv_slot slots[2];struct vv_work scratch;};
/* Zero initialized service; serialized owner; input/output/scratch disjoint.
 * Two provisioned client indexes, strictly increasing nonzero sessions and IDs.
 * Admission copies and verifies the bounded container before publishing.
 * No host capability: cancellation never abandons external work. */
int sfv_open(struct sfv_service *,unsigned,uint64_t);
int sfv_submit(struct sfv_service *,unsigned,const struct sfv_request *);
int sfv_step(struct sfv_service *,unsigned);
int sfv_cancel(struct sfv_service *,unsigned,uint64_t,uint64_t);
int sfv_reply(struct sfv_service *,unsigned,uint64_t,uint64_t,void *,size_t,size_t *,unsigned *);
int sfv_handle(struct sfv_service *,unsigned,struct sfv_handle *);
int sfv_field(struct sfv_service *,unsigned,const struct sfv_handle *,int,struct cvm_value *);
#endif
