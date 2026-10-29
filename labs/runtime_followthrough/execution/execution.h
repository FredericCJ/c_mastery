#ifndef CMASTERY_VM_EXECUTION_H
#define CMASTERY_VM_EXECUTION_H
#include "verifier.h"

enum
{
    VS_RUNNING = 32,
    VS_PENDING,
    VS_DONE,
    VS_STATE,
    VS_UNIMPLEMENTED
};

enum
{
    VS_CHECKED_V1,
    VS_WRAP_V2,
    VS_TAIL = 24
};

struct vs_request
{
    uint32_t epoch, serial;
    uint8_t service;
    int32_t argument;
};

struct vg_vs
{
    struct cvm_cvm vm;
    struct cvm_program image;
    uint32_t epoch, serial, capabilities, invocation[CVM_FRAMES],
        next_invocation;
    unsigned state, reason, numeric_mode, taken;
    struct vs_request request;
};

/* Start from zero-initialized or terminal state, never a pending/running one.
 * Owns copied image. Scratch is borrowed only during start. Failure preserves
 * the whole session. Epoch/serial/invocation counters refuse wrap. Disjoint
 * live objects, serialized calls, and no external mutation of live state.
 * TAIL24 is CALL-width followed by RET, requires exactly argc relative slots;
 * execution replaces current frame. Explicit API dialect, not ordinary CVM1.
 */
int vs_start(struct vg_vs *, const struct cvm_program *, unsigned mode,
             uint32_t fuel, uint32_t gc, uint32_t host, uint32_t capabilities,
             struct vv_work *);
/* Executes at most one guest instruction. Only terminal successful RET writes
 * result. Failures are sticky and diagnostic VM state may change. No C callback
 * is invoked: HOST yields a request and retains the continuation. */
int vs_step(struct vg_vs *, struct cvm_value *result);
/* Request may be taken once. Completion requires matching epoch+serial and
 * prior take. Caller keeps session alive until actual provider quiescence;
 * this serialized protocol does not implement OS I/O cancellation. */
int vs_take(struct vg_vs *, struct vs_request *);
int vs_complete(struct vg_vs *, uint32_t epoch, uint32_t serial, int failed,
                int32_t value);
int vs_arithmetic(unsigned mode, unsigned opcode, int32_t a, int32_t b,
                  int32_t *out);
#endif
