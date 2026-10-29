#ifndef CMASTERY_NOTIFICATIONS_DEPTH_H
#define CMASTERY_NOTIFICATIONS_DEPTH_H
#include "cmx.h"

enum
{
    CDN_IDLE,
    CDN_PENDING,
    CDN_DONE
};

enum
{
    CDN_STALE = CMX_UNIMPLEMENTED + 1,
    CDN_FAULT_PENDING,
    CDN_RETIRED
};

struct cdn_fault
{
    uint64_t sequence;
    uint32_t count;
    int reason;
};

struct cd_notify
{
    cmx_kernel *kernel;
    unsigned task, generation, maximum, attached, seen, closed;
    unsigned phase;
    uint32_t requested, claimed;
    int result;
    uint64_t accepted, consumed, cancelled, rejected, fault_sequence;
    struct cdn_fault fault;
    unsigned fault_pending;
};

struct cdn_handle
{
    struct cd_notify *owner;
    unsigned generation;
};

struct cdn_post
{
    unsigned woke, defer_schedule, object_probes;
};

void cdn_init(struct cd_notify *, cmx_kernel *);
int cdn_attach(struct cd_notify *, unsigned task, unsigned maximum,
               struct cdn_handle *);
int cdn_post(struct cdn_handle, uint32_t count, struct cdn_post *);
int cdn_wait(struct cdn_handle, uint32_t up_to, cmx_deadline);
int cdn_result(struct cdn_handle, uint32_t *count);
int cdn_cancel(struct cdn_handle);
int cdn_close(struct cdn_handle);
int cdn_detach(struct cdn_handle);
int cdn_fault(struct cdn_handle, struct cdn_fault *);
int cdn_ack_fault(struct cdn_handle, uint64_t sequence);
int cdn_invariant(const struct cd_notify *);
#endif
