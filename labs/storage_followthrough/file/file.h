#ifndef STORAGE_FOLLOW_FILE_H
#define STORAGE_FOLLOW_FILE_H
#include "store_file.h"
#include <stddef.h>
#include <stdint.h>
enum {
  SFI_OK = 0,
  SFI_INVALID,
  SFI_FULL,
  SFI_IO,
  SFI_UNCERTAIN,
  SFI_EXISTS,
  SFI_BUSY,
  SFI_WAIT,
  SFI_BUDGET,
  SFI_DONE,
  SFI_CANCELLED,
  SFI_STALE,
  SFI_UNIMPLEMENTED
};
enum { SFI_INTR = -2, SFI_AGAIN = -3 };
typedef struct {
  void *context;
  ptrdiff_t (*read)(void *, uint64_t, void *, size_t);
  ptrdiff_t (*write)(void *, uint64_t, const void *, size_t);
  uint32_t pages;
} SfiIo;
typedef struct SfiOp {
  SfiIo io;
  uint64_t generation, offset;
  size_t done;
  unsigned state, writing;
  unsigned char bytes[4096];
  void *output;
} SfiOp;
typedef struct {
  SfiOp *owner;
  uint64_t generation;
} SfiTicket;
/* Fresh/nonowning storage; thereafter stable, live, serialized and unmodified.
 * Byte callbacks are synchronous, nonreentrant and retain no buffer. Positive
 * count <=request accepts that prefix; INTR has no effect; AGAIN needs a fresh
 * external readiness observation; zero/other negative terminate. A spurious
 * readiness indication may still produce AGAIN. No implicit flush promise. */
void sfi_init(SfiOp *);
int sfi_begin(SfiOp *, SfiIo, unsigned writing, uint32_t page,
              const void *input, void *output, SfiTicket *);
int sfi_step(SfiTicket, unsigned ready, unsigned budget);
int sfi_cancel(SfiTicket);
int sfi_release(SfiTicket);
/* Linux profile: exclusive private dirfd namespace; simple1..32byte component;
 * fresh output owns no descriptor. Success owns a formatted/preallocated file
 * after file+directory fsync. Failure after creation leaves named evidence and
 * returns UNCERTAIN with output unchanged. No automatic unlink/retry. */
int sfi_create(int dirfd, const char *name, uint32_t pages, store_file *output);
int sfi_close(store_file *);
#endif
