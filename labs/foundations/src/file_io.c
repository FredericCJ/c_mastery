#define _POSIX_C_SOURCE 200809L
#include "foundation.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
struct f_file {
  int fd;
  unsigned calls;
};
static ptrdiff_t write_file(void *ctx, const uint8_t *data, size_t n) {
  struct f_file *f = ctx;
  ssize_t written;
  f->calls++;
  written = write(f->fd, data, n);
  return written < 0 ? (errno == EINTR ? -2 : -1) : (ptrdiff_t)written;
}
static uint64_t ns(struct timespec t) {
  return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}
int main(void) {
  char path[] = "/tmp/cmastery-io-XXXXXX";
  uint8_t source[4096], observed[4096];
  size_t accepted = 0, read_bytes = 0;
  struct timespec begin, after_write, after_sync;
  struct f_file f;
  enum f_status status;
  for (size_t i = 0; i < sizeof source; i++)
    source[i] = (uint8_t)i;
  f.fd = mkstemp(path);
  f.calls = 0;
  if (f.fd < 0)
    return 2;
  if (unlink(path)) {
    close(f.fd);
    return 2;
  }
  if (clock_gettime(CLOCK_MONOTONIC, &begin)) {
    close(f.fd);
    return 2;
  }
  status = f_write_all(write_file, &f, source, sizeof source, 16, &accepted);
  if (status != F_OK) {
    fprintf(stderr, "%s accepted=%zu\n", f_status_name(status), accepted);
    close(f.fd);
    return 1;
  }
  if (clock_gettime(CLOCK_MONOTONIC, &after_write) || fsync(f.fd) ||
      clock_gettime(CLOCK_MONOTONIC, &after_sync) ||
      lseek(f.fd, 0, SEEK_SET) < 0) {
    close(f.fd);
    return 2;
  }
  while (read_bytes < sizeof observed) {
    ssize_t got =
        read(f.fd, observed + read_bytes, sizeof observed - read_bytes);
    if (got < 0 && errno == EINTR)
      continue;
    if (got <= 0) {
      close(f.fd);
      return 2;
    }
    read_bytes += (size_t)got;
  }
  for (size_t i = 0; i < sizeof source; i++)
    if (source[i] != observed[i]) {
      close(f.fd);
      return 1;
    }
  if (close(f.fd))
    return 2;
  printf("{\"requested\":4096,\"accepted\":%zu,\"read_back\":%zu,\"write_"
         "calls\":%u,\"write_ns\":%llu,\"requested_fsync_ns\":%llu,\"claim\":"
         "\"same-file byte equality and successful fsync request; no "
         "crash-durability claim\"}\n",
         accepted, read_bytes, f.calls,
         (unsigned long long)(ns(after_write) - ns(begin)),
         (unsigned long long)(ns(after_sync) - ns(after_write)));
  return 0;
}
