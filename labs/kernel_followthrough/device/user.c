#include "kf_abi.h"
static unsigned char page[4096] __attribute__((aligned(4096)));
static volatile unsigned initialized = 1;
static void say(const char *p) {
  size_t n = 0;
  while (p[n])
    ++n;
  cmk_write(p, n);
}
static _Noreturn void done(int code) {
  cmk_call(CMK_EXIT, (uint64_t)code, 0, 0);
  for (;;) {
  }
}
__attribute__((section(".text.start"), noreturn)) void _start(void) {
  if (initialized != 1)
    done(20);
  int verify = cmk_call(CMK_BOOT_MODE, 0, 0, 0) == 1;
  void *handle = (void *)(uintptr_t)CMK_DISK_HANDLE;
  if (cmk_call(CMK_BLOCK_COUNT, CMK_DISK_HANDLE, 0, 0) != 1024)
    done(21);
  static const uint32_t pages[] = {0, 82, 1023};
  for (unsigned p = 0; p < 3; ++p) {
    if (verify) {
      if (cmk_block_read(handle, pages[p], page))
        done(22);
      for (unsigned i = 0; i < 4096; ++i)
        if (page[i] != (unsigned char)(pages[p] * 17 + i * 13 + 7))
          done(23);
    } else {
      for (unsigned i = 0; i < 4096; ++i)
        page[i] = (unsigned char)(pages[p] * 17 + i * 13 + 7);
      if (cmk_block_write(handle, pages[p], page))
        done(24);
    }
  }
  if (cmk_block_read(handle, 1024, page) != -1)
    done(25);
  if (!verify && KF_CASE == 31 && cmk_block_flush(handle))
    done(26);
  if (verify)
    say("KF:PASS:FRESH_GUEST_ALL_12288_PATTERN_BYTES\n");
  else if (KF_CASE == 31)
    say("KF:PASS:WRITE_COMPLETED_THEN_FLUSH_CONFIRMED\n");
  else
    say("KF:OBSERVATION:NO_FLUSH_WRITE_COMPLETED_NO_DURABILITY_CLAIM\n");
  if (cmk_call(CMK_CLOSE, CMK_DISK_HANDLE, 0, 0))
    done(27);
  done(0);
}
