#include "probe.h"
#include <stdio.h>
#include <string.h>
static struct kf_verifier_workspace workspace;
static struct cvm_cvm machine;
static struct cvm_program program;
static unsigned refused;
static int callback(void *unused, uint8_t service, int32_t input,
                    int32_t *out) {
  (void)unused;
  struct cvm_cvm before = machine;
  struct cvm_value nested = cvm_int(-91);
  if (service != 1 || input != 41 ||
      kf_run_scratch(&workspace, &machine, &program, &nested) != CVM_RANGE ||
      memcmp(&before, &machine, sizeof machine) || nested.integer != -91)
    return -1;
  ++refused;
  *out = 42;
  return 0;
}
int main(void) {
  static const uint8_t bytes[] = {'C', 'V', 'M',     '1', 1,      0,  0,
                                  8,   0,   0,       0,   OP_INT, 41, 0,
                                  0,   0,   OP_HOST, 1,   OP_RET};
  if (kf_load_scratch(0, bytes, sizeof bytes, &program) != CVM_RANGE ||
      kf_load_scratch(&workspace, bytes, sizeof bytes, &program) != CVM_OK)
    return 1;
  cvm_init(&machine, 32, 128, 1);
  machine.host = callback;
  struct cvm_value out = cvm_int(-99);
  if (kf_run_scratch(&workspace, &machine, &program, &out) != CVM_OK ||
      out.integer != 42 || refused != 1)
    return 2;
  /* Borrow released after success and after verifier error. */
  if (kf_load_scratch(&workspace, bytes, sizeof bytes - 1, &program) !=
          CVM_FORMAT ||
      kf_load_scratch(&workspace, bytes, sizeof bytes, &program) != CVM_OK)
    return 3;
  puts("KF:PASS:SCRATCH_REENTRY_REFUSED_UNCHANGED_RELEASED");
  return 0;
}
