#ifndef CMK_ABI_H
#define CMK_ABI_H
#include <stddef.h>
#include <stdint.h>
/* CMK ABI v1, x86-64 only. int 0x80; RAX number/result; RDI/RSI/RDX
 * arguments. Other general registers preserved. Negative result is an error.
 * One process owns disk capability 0x10001. No POSIX or libc ABI is implied. */
enum {
  CMK_WRITE = 1,
  CMK_EXIT = 2,
  CMK_YIELD = 3,
  CMK_TICKS = 4,
  CMK_ALLOC = 5,
  CMK_BLOCK_READ = 6,
  CMK_BLOCK_WRITE = 7,
  CMK_BLOCK_FLUSH = 8,
  CMK_BLOCK_COUNT = 9,
  CMK_READ = 10,
  CMK_BOOT_MODE = 11,
  CMK_BOOT_FILE = 12,
  CMK_CLOSE = 13,
  CMK_PID = 14
};
#define CMK_DISK_HANDLE UINT32_C(0x10001)
#define CMK_PAGE_SIZE 4096u
static inline int64_t cmk_call(uint64_t n, uint64_t a, uint64_t b, uint64_t c) {
  register uint64_t rax __asm__("rax") = n;
  __asm__ volatile("int $0x80"
                   : "+a"(rax)
                   : "D"(a), "S"(b), "d"(c)
                   : "memory", "cc");
  return (int64_t)rax;
}
static inline int64_t cmk_write(const void *p, size_t n) {
  return cmk_call(CMK_WRITE, (uintptr_t)p, n, 0);
}
static inline void *cmk_alloc(unsigned n) {
  int64_t r = cmk_call(CMK_ALLOC, n, 0, 0);
  return r < 0 ? NULL : (void *)(uintptr_t)r;
}
static inline int cmk_block_read(void *ctx, uint32_t page, void *p) {
  return (int)cmk_call(CMK_BLOCK_READ, (uintptr_t)ctx, page, (uintptr_t)p);
}
static inline int cmk_block_write(void *ctx, uint32_t page, const void *p) {
  return (int)cmk_call(CMK_BLOCK_WRITE, (uintptr_t)ctx, page, (uintptr_t)p);
}
static inline int cmk_block_flush(void *ctx) {
  return (int)cmk_call(CMK_BLOCK_FLUSH, (uintptr_t)ctx, 0, 0);
}
#endif
