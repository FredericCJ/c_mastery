#include "fixture.h"
#include "probe.h"
#include <stdio.h>
int main(int argc, char **argv) {
  if (argc == 2) {
    FILE *f = fopen(argv[1], "wb");
    if (!f)
      return 8;
    size_t n = fwrite(kf_vm_bytes, 1, sizeof kf_vm_bytes, f);
    int closed = fclose(f);
    if (n != sizeof kf_vm_bytes || closed)
      return 9;
  }
  struct kf_vm_report r;
  int code = kf_vm_probe(kf_vm_bytes, sizeof kf_vm_bytes, &r);
  if (code) {
    printf("KF_VM_FAIL:%d\n", code);
    return code;
  }
  for (unsigned i = 0; i < 6; ++i)
    printf("KF_VM:%u:%u:%d:%u:%u:%u:%u\n", i, r.cases[i].status,
           r.cases[i].value, r.cases[i].calls, r.cases[i].loop_iterations,
           r.cases[i].fuel_left, r.cases[i].host_left);
  printf("KF_VM_TAIL:%u:%u:%u:%zu:%zu:%zu\n", r.truncated, r.recursive,
         r.run_calls, r.machine_bytes, r.program_bytes, r.workspace_bytes);
  return 0;
}
