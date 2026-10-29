/* Included only into an isolated copy of the original CMK2 user workload. */
#include "async.h"
#define SA_CHECK(x) do { if (!(x)) { message("SA_GUEST_CHECK " #x "\n"); die(1); } } while (0)
static uint8_t sa_bytes[4096];
static int64_t sa_submit_guest(void *ctx, uint32_t page, void *buffer) {
  struct cmk2_block_request r = {0, page, (uintptr_t)buffer};
  return cmk_call(CMK2_BLOCK_SUBMIT, (uintptr_t)ctx, (uintptr_t)&r, 0);
}
static int64_t sa_wait_guest(void *ctx, uint64_t ticket) {
  (void)ctx;
  return cmk_call(CMK2_BLOCK_WAIT, ticket, 0, 0);
}
static void sa_guest(void) {
  void *h = (void *)(uintptr_t)CMK_DISK_HANDLE;
  for (unsigned i = 0; i < 4096; i++)
    sa_bytes[i] = (uint8_t)(i * 13 + 7);
  SA_CHECK(!cmk2_block(h, 1, 8, sa_bytes));
  SA_CHECK(!cmk2_block(h, 4, 0, 0));
  for (unsigned i = 0; i < 4096; i++)
    sa_bytes[i] = 0;
  struct sa_read operation = {0};
  struct sa_port port = {h, sa_submit_guest, sa_wait_guest};
  void *out = h;
  SA_CHECK(sa_start(&operation, port, 8, sa_bytes) == SA_OK);
  SA_CHECK(sa_abandon(&operation) == SA_BUSY);
  SA_CHECK(sa_release(&operation, &out) == SA_BUSY && out == h);
  message("SA_GUEST retained-after-abandon-before-positive-wait\n");
  SA_CHECK(sa_finish(&operation) == SA_DRAINED);
  for (unsigned i = 0; i < 4096; i++)
    SA_CHECK(sa_bytes[i] == (uint8_t)(i * 13 + 7));
  SA_CHECK(sa_release(&operation, &out) == SA_OK && out == sa_bytes);
  for (unsigned i = 0; i < 4096; i++)
    sa_bytes[i] = 0xdd;
  SA_CHECK(sa_start(&operation, port, 8, sa_bytes) == SA_OK);
  SA_CHECK(sa_finish(&operation) == SA_READY);
  for (unsigned i = 0; i < 4096; i++)
    SA_CHECK(sa_bytes[i] == (uint8_t)(i * 13 + 7));
  SA_CHECK(sa_release(&operation, &out) == SA_OK);
  message("SA_GUEST_PASS actual-CMK2-read-borrow-completion-before-reuse "
          "bytes4096\n");
}
