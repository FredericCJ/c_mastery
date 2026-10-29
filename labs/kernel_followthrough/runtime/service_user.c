#include "cvm.h"
#include "device.h"
#include "kf_abi.h"
#include "services.h"
static store database;
static struct cvm_cvm machine;
static struct cvm_program program;
static struct cvm_value result;
static struct vg_vh owner, reader;
static unsigned lose_reply, refuse_before;
static void say(const char *p) {
  size_t n = 0;
  while (p[n])
    ++n;
  cmk_write(p, n);
}
static _Noreturn void done(int code) {
  if (code)
    say("KF:SERVICE_FAILURE\n");
  cmk_call(CMK_EXIT, (uint64_t)code, 0, 0);
  for (;;) {
  }
}
static int read_page(void *ctx, uint32_t logical, void *bytes) {
  uint32_t physical;
  if (!kf_slice(1024, 16, 67, logical, &physical))
    return -1;
  return cmk_block_read(ctx, physical, bytes);
}
static int write_page(void *ctx, uint32_t logical, const void *bytes) {
  uint32_t physical;
  if (!kf_slice(1024, 16, 67, logical, &physical))
    return -1;
  return cmk_block_write(ctx, physical, bytes);
}
static int host(void *ctx, uint8_t service, int32_t input, int32_t *out) {
  if (refuse_before)
    return STORE_IO;
  int r = vh_host(ctx, service, input, out);
  return r ? r : lose_reply ? STORE_INDETERMINATE : 0;
}
static enum cvm_status execute(struct vg_vh *context, unsigned service, int32_t input) {
  uint32_t u = (uint32_t)input;
  uint8_t bytes[] = {'C',
                     'V',
                     'M',
                     '1',
                     1,
                     0,
                     0,
                     8,
                     0,
                     0,
                     0,
                     OP_INT,
                     (uint8_t)u,
                     (uint8_t)(u >> 8),
                     (uint8_t)(u >> 16),
                     (uint8_t)(u >> 24),
                     OP_HOST,
                     (uint8_t)service,
                     OP_RET};
  if (cvm_load(bytes, sizeof bytes, &program) != CVM_OK)
    done(30);
  cvm_init(&machine, 32, 128, 1);
  machine.host = host;
  machine.host_context = context;
  result = cvm_int(-99);
  return cvm_run(&machine, &program, &result);
}
__attribute__((section(".text.start"), noreturn)) void _start(void) {
  int verify = cmk_call(CMK_BOOT_MODE, 0, 0, 0) == 1;
  store_io io = {(void *)(uintptr_t)CMK_DISK_HANDLE, read_page, write_page,
                 cmk_block_flush, 67};
  if (!verify && store_format(io) != STORE_OK)
    done(31);
  if (store_open(&database, io) != STORE_OK)
    done(32);
  owner = (struct vg_vh){&database, 14, KF_CASE == 33 ? VH_LEGACY4 : VH_VERSIONED5, 0, 1};
  reader = owner;
  reader.capabilities = 4;
  if (!verify) {
    if (KF_CASE == 33) {
      lose_reply = 1;
      if (execute(&owner, 1, 42) != CVM_HOST_ERROR || result.integer != -99)
        done(33);
      lose_reply = 0;
      say("KF:PASS:COMMITTED_ASSIGNMENT_LOST_REPLY_HOST_ERROR\n");
    } else {
      if (execute(&owner, 1, 41) != CVM_OK || result.integer != 41)
        done(34);
      lose_reply = KF_CASE == 34;
      refuse_before = KF_CASE == 35;
      if (execute(&owner, 3, 1) != CVM_HOST_ERROR || result.integer != -99)
        done(35);
      lose_reply = refuse_before = 0;
      say(KF_CASE == 34 ? "KF:PASS:COMMITTED_INCREMENT_LOST_REPLY_HOST_ERROR\n"
                        : "KF:PASS:PRECOMMIT_REFUSAL_SAME_HOST_ERROR\n");
    }
  } else {
    int32_t expected = KF_CASE == 35 ? 41 : 42;
    if (execute(&reader, 2, 0) != CVM_OK || result.integer != expected)
      done(36);
    say(KF_CASE == 35 ? "KF:PASS:DURABLE_QUERY_41_BEFORE_RETRY\n"
                      : "KF:PASS:DURABLE_QUERY_42_AFTER_LOST_REPLY\n");
    if (KF_CASE != 33) {
      uint64_t generation = database.generation;
      if (execute(&owner, 3, 1) != CVM_OK || result.integer != 42)
        done(37);
      if (KF_CASE == 34 && database.generation != generation)
        done(38);
      generation = database.generation;
      if (execute(&owner, 3, 1) != CVM_OK || result.integer != 42 ||
          database.generation != generation)
        done(39);
      if (execute(&owner, 3, 2) != CVM_HOST_ERROR ||
          database.generation != generation)
        done(40);
      owner.operation_id = 3;
      if (execute(&owner, 3, 1) != CVM_HOST_ERROR ||
          database.generation != generation)
        done(41);
      say("KF:PASS:ATOMIC_RECEIPT_DUPLICATE_NO_SECOND_INCREMENT_CHANGED_"
          "PAYLOAD_GAP_REFUSED\n");
    }
  }
  uint64_t reads = database.reads, writes = database.writes,
           flushes = database.flushes;
  if (execute(&reader, 1, 91) != CVM_HOST_ERROR || result.integer != -99 ||
      database.reads != reads || database.writes != writes ||
      database.flushes != flushes)
    done(42);
  if (store_check(&database) != STORE_OK)
    done(43);
  if (execute(&reader, 2, 0) != CVM_OK ||
      result.integer != (KF_CASE == 35 && !verify ? 41 : 42))
    done(44);
  say("KF:PASS:READONLY_CLIENT_SERIALIZED_EXCLUSIVE_STORE_OWNER\nKF:PASS:CVM_"
      "SERVICE_STORE_PAGE_SYSCALL_VIRTIO_CHAIN\n");
  if (cmk_call(CMK_CLOSE, CMK_DISK_HANDLE, 0, 0))
    done(45);
  done(0);
}
