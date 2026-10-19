#ifndef KF_RUNTIME_PROBE_H
#define KF_RUNTIME_PROBE_H
#include "cvm.h"
typedef struct {
  uint8_t boundary[CVM_CODE];
  int16_t height[CVM_CODE];
  uint16_t queue[CVM_CODE];
} KfVerifierWorkspace;
typedef struct {
  CvmStatus status;
  int32_t value;
  unsigned calls, loop_iterations;
  uint32_t fuel_left, host_left;
} KfVmResult;
typedef struct {
  KfVmResult cases[6];
  CvmStatus truncated, recursive;
  unsigned run_calls;
  size_t machine_bytes, program_bytes, workspace_bytes;
} KfVmReport;
CvmStatus kf_load_scratch(KfVerifierWorkspace *, const uint8_t *, size_t,
                          CvmProgram *);
CvmStatus kf_run_scratch(KfVerifierWorkspace *, Cvm *, const CvmProgram *,
                         CvmValue *);
int kf_vm_probe(const uint8_t *, size_t, KfVmReport *);
#endif
