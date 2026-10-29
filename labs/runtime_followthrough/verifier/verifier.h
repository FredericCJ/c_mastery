#ifndef CMASTERY_RUNTIME_VERIFIER_H
#define CMASTERY_RUNTIME_VERIFIER_H
#include "cvm.h"

enum
{
    VV_OK,
    VV_ARGUMENT,
    VV_LAYOUT,
    VV_DECODE,
    VV_TARGET,
    VV_HEIGHT,
    VV_INITIALIZED,
    VV_SPACE,
    VV_UNIMPLEMENTED
};

/* Explicit verifier dialect selector:0=CVM1,1=CVM1 plus SPLIT23.
 * SPLIT(width5) pops a Boolean and selects trueLE16 at+1 / falseLE16 at+3.
 * No fallthrough. Only vv_lower output may run on the unchanged CVM1 VM. */
enum
{
    VV_SPLIT = 23
};

struct vv_work
{
    uint8_t boundary[CVM_CODE], initialized[CVM_CODE], queued[CVM_CODE];
    int16_t height[CVM_CODE];
    uint16_t queue[CVM_CODE], relocation[CVM_CODE + 1];
};

struct vv_report
{
    unsigned phase, pc, decoded, discovered, processed, edges, local_changes,
        max_queue;
};

/* Scratch and report may change on failure; program inputs are immutable.
 * Caller owns disjoint live workspace/report/input objects, serialized calls.
 * Definite initialization joins use intersection; params initialized at entry.
 * Unreachable instructions still require valid widths, operands and targets.
 * Stack/init transfer checks apply only to reachable instructions per function.
 */
int vv_verify(const struct cvm_program *, unsigned dialect, unsigned definite,
              struct vv_work *, struct vv_report *);
/* Failure-atomic program publication, report retains earliest phase diagnosis.
 * Raw CVM1 container can carry dialect1 instructions only under the explicitly
 * selected verifier contract; raw bytes alone never authorize dispatch. */
int vv_load(const uint8_t *, size_t, unsigned, unsigned, struct vv_work *,
            struct cvm_program *, struct vv_report *);
/* Lower validated SPLIT into JFALSE false; JMP true and relocate every original
 * branch/descriptor. Requires output length<=1024; output unchanged on refusal.
 * Output has ordinary CVM1 semantics and is checked by actual cvm_verify. */
int vv_lower(const struct cvm_program *, unsigned, struct vv_work *,
             struct cvm_program *, struct vv_report *);
#endif
