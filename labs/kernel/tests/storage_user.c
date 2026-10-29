#include "cmk_abi.h"
#include "cvm.h"
#include "store.h"
#include "store_host.h"
static store database;
static struct cvm_cvm vm;
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
void *memmove(void *d, const void *s, size_t n) {
  unsigned char *p = d;
  const unsigned char *q = s;
  /* This adapter is restricted to the fixed flat-address x86-64 guest. */
  if ((uintptr_t)p < (uintptr_t)q)
    while (n--)
      *p++ = *q++;
  else
    while (n) {
      n--;
      p[n] = q[n];
    }
  return d;
}
int memcmp(const void *a, const void *b, size_t n) {
  const unsigned char *p = a, *q = b;
  while (n--) {
    if (*p != *q)
      return *p < *q ? -1 : 1;
    p++;
    q++;
  }
  return 0;
}
static int read_page(void *ctx, uint32_t p, void *out) {
  if (p >= STORE_IMAGE_PAGES)
    return -1;
  return cmk_block_read(ctx, p + 16, out);
}
static int write_page(void *ctx, uint32_t p, const void *in) {
  if (p >= STORE_IMAGE_PAGES)
    return -1;
  return cmk_block_write(ctx, p + 16, in);
}
static int lost_reply(void *ctx, uint8_t service, int32_t input,
                      int32_t *output) {
  int r = cvm_store_host(ctx, service, input, output);
  return r ? r : (service == 1 ? STORE_INDETERMINATE : 0);
}
int storage_test(int verify) {
  store_io io = {(void *)(uintptr_t)CMK_DISK_HANDLE, read_page, write_page,
                 cmk_block_flush, STORE_IMAGE_PAGES};
  if (!verify && store_format(io) != STORE_OK)
    return 1;
  if (store_open(&database, io) != STORE_OK)
    return 2;
  uint8_t bytes[] = {'C', 'V',    'M', '1', 1, 0, 0,       8, 0,     0,
                     0,   OP_INT, 42,  0,   0, 0, OP_HOST, 1, OP_RET};
  if (verify)
    bytes[17] = 2;
  if (cvm_load(bytes, sizeof bytes, &program) != CVM_OK)
    return 3;
  cvm_init(&vm, 32, 128, 1);
  vm.host_context = &database;
  vm.host = verify ? cvm_store_host : lost_reply;
  enum cvm_status r = cvm_run(&vm, &program, &result);
  if (verify) {
    if (r != CVM_OK || result.tag != CVM_INT || result.integer != 42)
      return 4;
    static const char s[] = "PASS VM-store-recovery-after-reboot\n";
    cmk_write(s, sizeof s - 1);
  } else {
    if (r != CVM_HOST_ERROR)
      return 5;
    static const char s[] = "PASS VM-store-commit-lost-response\n";
    cmk_write(s, sizeof s - 1);
  }
  if (store_check(&database) != STORE_OK)
    return 6;
  return 0;
}
