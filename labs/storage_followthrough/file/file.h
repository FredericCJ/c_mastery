#ifndef STORAGE_FOLLOW_FILE_H
#define STORAGE_FOLLOW_FILE_H
#include "store_file.h"
#include <stddef.h>
#include <stdint.h>

enum
{
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

enum
{
    SFI_INTR = -2,
    SFI_AGAIN = -3
};

struct sfi_io
{
    void *context;
    ptrdiff_t (*read)(void *, uint64_t, void *, size_t);
    ptrdiff_t (*write)(void *, uint64_t, const void *, size_t);
    uint32_t pages;
};

struct sfi_op
{
    struct sfi_io io;
    uint64_t generation, offset;
    size_t done;
    unsigned state, writing;
    unsigned char bytes[4096];
    void *output;
};

struct sfi_ticket
{
    struct sfi_op *owner;
    uint64_t generation;
};

/* Fresh/nonowning storage; thereafter stable, live, serialized and unmodified.
 * Byte callbacks are synchronous, nonreentrant and retain no buffer. Positive
 * count <=request accepts that prefix; INTR has no effect; AGAIN needs a fresh
 * external readiness observation; zero/other negative terminate. A spurious
 * readiness indication may still produce AGAIN. No implicit flush promise. */
void sfi_init(struct sfi_op *);
int sfi_begin(struct sfi_op *, struct sfi_io, unsigned writing, uint32_t page,
              const void *input, void *output, struct sfi_ticket *);
int sfi_step(struct sfi_ticket, unsigned ready, unsigned budget);
int sfi_cancel(struct sfi_ticket);
int sfi_release(struct sfi_ticket);
/* Linux profile: exclusive private dirfd namespace; simple1..32byte component;
 * fresh output owns no descriptor. Success owns a formatted/preallocated file
 * after file+directory fsync. Failure after creation leaves named evidence and
 * returns UNCERTAIN with output unchanged. No automatic unlink/retry. */
int sfi_create(int dirfd, const char *name, uint32_t pages, store_file *output);
int sfi_close(store_file *);
#endif
