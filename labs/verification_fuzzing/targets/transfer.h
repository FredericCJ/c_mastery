#ifndef CM_VF_TRANSFER_H
#define CM_VF_TRANSFER_H
#include <stddef.h>
#include <stdint.h>
int vt_mul(uint32_t,uint32_t,uint32_t*);
int vt_narrow(int64_t,uint16_t*);
typedef struct {int32_t key; unsigned identity;} VTRecord;
size_t vt_desc(const VTRecord*,size_t,int32_t);
typedef void (*VTNotify)(void*,const void*);
typedef int (*VTPrepare)(void*,const void*);
typedef struct {const void *payload[3];VTNotify notify[3];void *context[3];unsigned n,busy;} VTQueue;
int vt_offer(VTQueue*,const void*,VTPrepare,VTNotify,void*);
int vt_remove(VTQueue*);
typedef struct {unsigned char bytes[18];size_t n;unsigned rejected;} VTStream;
void vt_reset(VTStream*);
int vt_feed(VTStream*,const unsigned char*,size_t);
int vt_finish(const VTStream*,uint32_t*);
/* Events retain stable identities when deleted. op0 submit,1 cancel,2 complete,
 * 3 faulty release,4 irrelevant. Signature names first premature release. */
typedef struct {unsigned id,op;} VTEvent;
unsigned vt_violation(const VTEvent*,size_t);
size_t vt_shrink(VTEvent*,size_t);
#endif
