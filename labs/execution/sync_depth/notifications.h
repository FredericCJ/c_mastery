#ifndef CMASTERY_NOTIFICATIONS_DEPTH_H
#define CMASTERY_NOTIFICATIONS_DEPTH_H
#include "cmx.h"
enum { CDN_IDLE, CDN_PENDING, CDN_DONE };
enum { CDN_STALE = CMX_UNIMPLEMENTED + 1, CDN_FAULT_PENDING, CDN_RETIRED };
typedef struct {
  uint64_t sequence;
  uint32_t count;
  int reason;
} CDNFault;
typedef struct {
  cmx_kernel *kernel;
  unsigned task, generation, maximum, attached, seen, closed;
  unsigned phase;
  uint32_t requested, claimed;
  int result;
  uint64_t accepted, consumed, cancelled, rejected, fault_sequence;
  CDNFault fault;
  unsigned fault_pending;
} CDNotify;
typedef struct { CDNotify *owner; unsigned generation; } CDNHandle;
typedef struct { unsigned woke, defer_schedule, object_probes; } CDNPost;
void cdn_init(CDNotify *, cmx_kernel *);
int cdn_attach(CDNotify *, unsigned task, unsigned maximum, CDNHandle *);
int cdn_post(CDNHandle, uint32_t count, CDNPost *);
int cdn_wait(CDNHandle, uint32_t up_to, cmx_deadline);
int cdn_result(CDNHandle, uint32_t *count);
int cdn_cancel(CDNHandle);
int cdn_close(CDNHandle);
int cdn_detach(CDNHandle);
int cdn_fault(CDNHandle, CDNFault *);
int cdn_ack_fault(CDNHandle, uint64_t sequence);
int cdn_invariant(const CDNotify *);
#endif
