#include "cmk_abi.h"
#include "probe.h"
static uint8_t image[64];
static char name[24] = "bytecode";
static struct kf_vm_report report;
static void print(const char *p) {
  size_t n = 0;
  while (p[n])
    ++n;
  cmk_write(p, n);
}
static void number(uint64_t x) {
  char b[24];
  unsigned i = 0;
  do {
    b[i++] = (char)('0' + x % 10);
    x /= 10;
  } while (x);
  while (i) {
    --i;
    cmk_write(b + i, 1);
  }
}
static _Noreturn void end(int code) {
  cmk_call(CMK_EXIT, (uint64_t)code, 0, 0);
  for (;;) {
  }
}
__attribute__((section(".text.start"), noreturn)) void _start(void) {
  int64_t n =
      cmk_call(CMK_BOOT_FILE, (uintptr_t)name, (uintptr_t)image, sizeof image);
  if (n < 0)
    end(30);
  int code = kf_vm_probe(image, (size_t)n, &report);
  if (code) {
    print("KF_VM_FAIL:");
    number((unsigned)code);
    print("\n");
    end(31);
  }
  if (n < 19) {
    if (report.run_calls || report.truncated == CVM_OK)
      end(32);
    print("KF:PASS:ARCHIVE_TRUNCATION_BEFORE_VM_RUN\n");
    end(0);
  }
  for (unsigned i = 0; i < 6; ++i) {
    struct kf_vm_result *r = &report.cases[i];
    print("KF_VM:");
    number(i);
    print(":");
    number(r->status);
    print(":");
    if (r->value < 0) {
      print("-");
      number((uint64_t)-(int64_t)r->value);
    } else
      number((uint64_t)r->value);
    print(":");
    number(r->calls);
    print(":");
    number(r->loop_iterations);
    print(":");
    number(r->fuel_left);
    print(":");
    number(r->host_left);
    print("\n");
  }
  print("KF_VM_TAIL:");
  number(report.truncated);
  print(":");
  number(report.recursive);
  print(":");
  number(report.run_calls);
  print(":");
  number(report.machine_bytes);
  print(":");
  number(report.program_bytes);
  print(":");
  number(report.workspace_bytes);
  print("\nKF:PASS:VM_SHARED_CORE\n");
  end(0);
}
