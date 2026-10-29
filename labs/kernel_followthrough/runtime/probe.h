#ifndef KF_RUNTIME_PROBE_H
#define KF_RUNTIME_PROBE_H
#include "cvm.h"

struct kf_verifier_workspace
{
    uint8_t boundary[CVM_CODE];
    int16_t height[CVM_CODE];
    uint16_t queue[CVM_CODE];
};

struct kf_vm_result
{
    enum cvm_status status;
    int32_t value;
    unsigned calls, loop_iterations;
    uint32_t fuel_left, host_left;
};

struct kf_vm_report
{
    struct kf_vm_result cases[6];
    enum cvm_status truncated, recursive;
    unsigned run_calls;
    size_t machine_bytes, program_bytes, workspace_bytes;
};
enum cvm_status kf_load_scratch(struct kf_verifier_workspace *, const uint8_t *,
                                size_t, struct cvm_program *);
enum cvm_status kf_run_scratch(struct kf_verifier_workspace *, struct cvm_cvm *,
                               const struct cvm_program *, struct cvm_value *);
int kf_vm_probe(const uint8_t *, size_t, struct kf_vm_report *);
#endif
