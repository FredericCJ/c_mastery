#define _POSIX_C_SOURCE 200809L
#include "file.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#ifndef SFI_MUTANT
#define SFI_MUTANT 0
#endif
_Static_assert(sizeof(off_t) >= 8, "selected Linux64 file-offset profile");
void sfi_init(SfiOp *o) { memset(o, 0, sizeof *o); }
static int valid(SfiTicket t) {
  return t.owner && t.generation && t.owner->generation == t.generation &&
         t.owner->state;
}
int sfi_begin(SfiOp *o, SfiIo io, unsigned writing, uint32_t page,
              const void *input, void *output, SfiTicket *ticket) {
  if (!o || !ticket || writing > 1 || !io.pages || page >= io.pages ||
      (writing ? (!input || !io.write) : (!output || !io.read)))
    return SFI_INVALID;
  if (o->state)
    return SFI_BUSY;
  if (o->generation == UINT64_MAX)
    return SFI_FULL;
  o->io = io;
  o->offset = (uint64_t)page * 4096u;
  o->done = 0;
  o->writing = writing;
  o->output = output;
  o->generation++;
  o->state = 1;
  if (writing)
    memcpy(o->bytes, input, 4096);
  *ticket = (SfiTicket){o, o->generation};
  return SFI_OK;
}
int sfi_step(SfiTicket t, unsigned ready, unsigned budget) {
  if (!valid(t))
    return SFI_STALE;
  if (ready > 1)
    return SFI_INVALID;
  SfiOp *o = t.owner;
  if (o->state == 3)
    return SFI_DONE;
  if (o->state == 4)
    return SFI_IO;
  if (o->state == 5)
    return SFI_CANCELLED;
  if (o->state == 2 && !ready && SFI_MUTANT != 2)
    return SFI_WAIT;
  if (!budget)
    return SFI_BUDGET;
  o->state = 1;
  while (budget--) {
    size_t at = SFI_MUTANT == 1 ? 0 : o->done;
    size_t left = 4096 - o->done;
    ptrdiff_t n =
        o->writing
            ? o->io.write(o->io.context, o->offset + at, o->bytes + at, left)
            : o->io.read(o->io.context, o->offset + at, o->bytes + at, left);
    if (n == SFI_INTR)
      continue;
    if (n == SFI_AGAIN) {
      o->state = 2;
      return SFI_WAIT;
    }
    if (n <= 0 || (size_t)n > left) {
      o->state = 4;
      return SFI_IO;
    }
    o->done += (size_t)n;
    if (o->done == 4096) {
      if (!o->writing)
        memcpy(o->output, o->bytes, 4096);
      o->state = 3;
      return SFI_DONE;
    }
  }
  return SFI_BUDGET;
}
int sfi_cancel(SfiTicket t) {
  if (!valid(t))
    return SFI_STALE;
  if (t.owner->state >= 3)
    return SFI_BUSY;
  t.owner->state = 5;
  return SFI_CANCELLED;
}
int sfi_release(SfiTicket t) {
  if (!valid(t))
    return SFI_STALE;
  if (t.owner->state < 3)
    return SFI_BUSY;
  uint64_t generation = t.owner->generation;
  memset(t.owner, 0, sizeof *t.owner);
  t.owner->generation = generation;
  return SFI_OK;
}
static int syncfd(int fd) {
  int r;
  do {
    r = fsync(fd);
  } while (r < 0 && errno == EINTR);
  return r;
}
int sfi_close(store_file *f) {
  if (!f || f->fd < 0)
    return SFI_INVALID;
  int fd = f->fd;
  f->fd = -1;
  return close(fd) ? SFI_UNCERTAIN : SFI_OK;
}
int sfi_create(int dirfd, const char *name, uint32_t pages, store_file *out) {
  if (dirfd < 0 || !name || !out || pages < STORE_IMAGE_PAGES)
    return SFI_INVALID;
  size_t n = 0;
  while (name[n]) {
    unsigned char c = (unsigned char)name[n];
    if (n == 32 || !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                     (c >= '0' && c <= '9') || c == '-' || c == '_'))
      return SFI_INVALID;
    n++;
  }
  if (!n)
    return SFI_INVALID;
  int fd = openat(dirfd, name,
                  O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
  if (fd < 0)
    return errno == EEXIST ? SFI_EXISTS : SFI_IO;
  store_file built = {fd, pages};
  int failed = posix_fallocate(fd, 0, (off_t)pages * 4096) != 0;
  if (!failed)
    failed = store_format(store_file_io(&built)) != STORE_OK;
  if (!failed)
    failed = syncfd(fd) != 0;
  if (!failed && SFI_MUTANT != 3)
    failed = syncfd(dirfd) != 0;
  if (failed) {
    if (SFI_MUTANT == 4)
      *out = built;
    (void)sfi_close(&built);
    return SFI_UNCERTAIN;
  }
  *out = built;
  return SFI_OK;
}
