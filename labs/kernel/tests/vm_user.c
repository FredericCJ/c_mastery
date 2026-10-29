#include "cmk_abi.h"
#include "cvm.h"
static struct cvm_cvm machine;
static struct cvm_program program;
static struct cvm_value result;
void *memcpy(void *d, const void *s, size_t n) {
  unsigned char *p = d;
  const unsigned char *q = s;
  while (n--)
    *p++ = *q++;
  return d;
}
void *memset(void *d, int c, size_t n) {
  unsigned char *p = d;
  while (n--)
    *p++ = (unsigned char)c;
  return d;
}
static int host(void *ctx, uint8_t service, int32_t value, int32_t *out) {
  (void)ctx;
  if (service != 1 || value > 1000 || value < 0)
    return -1;
  *out = value + 1;
  return 0;
}
static void end(int code) {
  cmk_call(CMK_EXIT, (uint64_t)code, 0, 0);
  for (;;) {
  }
}
__attribute__((section(".text.start"), noreturn)) void _start(void) {
  static const uint8_t bytes[] = {'C', 'V', 'M',     '1', 1,      0,  0,
                                  8,   0,   0,       0,   OP_INT, 41, 0,
                                  0,   0,   OP_HOST, 1,   OP_RET};
  if (cvm_load(bytes, sizeof bytes, &program) != CVM_OK)
    end(21);
  cvm_init(&machine, 32, 128, 1);
  machine.host = host;
  if (cvm_run(&machine, &program, &result) != CVM_OK || result.tag != CVM_INT ||
      result.integer != 42)
    end(22);
  cvm_init(&machine, 32, 128, 0);
  machine.host = host;
  if (cvm_run(&machine, &program, &result) != CVM_HOST_BUDGET)
    end(23);
  static const char message[] = "PASS runtime-in-user-process\n";
  cmk_write(message, sizeof message - 1);
  end(0);
}
