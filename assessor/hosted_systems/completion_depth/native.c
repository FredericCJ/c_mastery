#define _GNU_SOURCE
#include "completion.h"
#include "raw_ring.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "COMPLETION-NATIVE-CHECK:%d:%s errno=%d\n", __LINE__,    \
              #x, errno);                                                      \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
typedef struct {
  CMRawRing ring;
  CoPool pool;
  unsigned char *map;
  int registered;
} Context;
static uint64_t now(void) {
  struct timespec t;
  CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
  return (uint64_t)t.tv_sec * 1000000000u + (unsigned long)t.tv_nsec;
}
static int reg(int fd, unsigned op, void *arg, unsigned n) {
  return (int)syscall(__NR_io_uring_register, fd, op, arg, n);
}
static void cleanup(Context *c) {
  if (c->registered) {
    CHECK(co_quiescent(&c->pool));
    CHECK(reg(c->ring.fd, IORING_UNREGISTER_BUFFERS, NULL, 0) == 0);
    c->registered = 0;
  }
  int fd = c->ring.fd;
  cm_ring_close(&c->ring);
  if (fd >= 0) {
    errno = 0;
    CHECK(fcntl(fd, F_GETFD) == -1 && errno == EBADF);
  }
  if (c->map) {
    CHECK(munmap(c->map, 16384) == 0);
    c->map = NULL;
  }
}
/* Every injected cut precedes publication. Post-publication shutdown is tested
 * separately by consuming both target and cancellation CQEs before cleanup. */
static int setup(Context *c, int fixed, int cut) {
  memset(c, 0, sizeof *c);
  c->ring.fd = -1;
  if (cut == 0)
    return -1;
  c->map = mmap(NULL, 16384, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  CHECK(c->map != MAP_FAILED);
  unsigned char *b[4];
  for (unsigned i = 0; i < 4; ++i)
    b[i] = c->map + 4096 * i;
  CHECK(co_init(&c->pool, b, 4096, 0x7fffffffu) == 0);
  if (cut == 1) {
    cleanup(c);
    return -1;
  }
  int result = cm_ring_init(&c->ring, 8);
  if (result) {
    fprintf(stderr, "COMPLETION-UNAVAILABLE:io_uring_setup=%d\n", result);
    cleanup(c);
    exit(77);
  }
  if (cut == 2) {
    cleanup(c);
    return -1;
  }
  size_t bytes =
      sizeof(struct io_uring_probe) + 256 * sizeof(struct io_uring_probe_op);
  struct io_uring_probe *probe = calloc(1, bytes);
  CHECK(probe);
  CHECK(reg(c->ring.fd, IORING_REGISTER_PROBE, probe, 256) == 0);
  unsigned found = 0;
  for (unsigned i = 0; i < probe->ops_len; ++i)
    if (probe->ops[i].flags & IO_URING_OP_SUPPORTED) {
      if (probe->ops[i].op == IORING_OP_READ)
        found |= 1;
      if (probe->ops[i].op == IORING_OP_READ_FIXED)
        found |= 2;
      if (probe->ops[i].op == IORING_OP_ASYNC_CANCEL)
        found |= 4;
    }
  free(probe);
  CHECK(found == 7);
  struct rlimit limit;
  CHECK(getrlimit(RLIMIT_MEMLOCK, &limit) == 0);
  printf(
      "COMPLETION-PROBE:READ=1;READ_FIXED=1;ASYNC_CANCEL=1;memlock_soft=%ju;\n",
      (uintmax_t)limit.rlim_cur);
  if (cut == 3) {
    cleanup(c);
    return -1;
  }
  if (fixed) {
    struct iovec v[4];
    for (unsigned i = 0; i < 4; ++i)
      v[i] = (struct iovec){b[i], 4096};
    if (reg(c->ring.fd, IORING_REGISTER_BUFFERS, v, 4)) {
      int e = errno;
      cleanup(c);
      fprintf(stderr, "COMPLETION-UNAVAILABLE:register=%d\n", e);
      exit(77);
    }
    c->registered = 1;
  }
  if (cut == 4) {
    cleanup(c);
    return -1;
  }
  return 0;
}
static uint64_t read_op(Context *c, unsigned slot, int fd, unsigned n,
                        int fixed, unsigned displacement) {
  uint64_t token;
  CHECK(co_acquire(&c->pool, slot, n, &token) == 1);
  struct io_uring_sqe s = {0};
  s.opcode = fixed ? IORING_OP_READ_FIXED : IORING_OP_READ;
  s.fd = fd;
  s.off = UINT64_MAX;
  s.addr = (uintptr_t)(c->pool.slot[slot].buffer + displacement);
  s.len = n;
  s.buf_index = (unsigned short)slot;
  s.user_data = token;
  CHECK(cm_ring_push(&c->ring, &s) == 0 && cm_ring_submit(&c->ring) == 0);
  return token;
}
static void cancel_op(Context *c, uint64_t target) {
  uint64_t token;
  CHECK(co_cancel(&c->pool, target, &token) == 1);
  struct io_uring_sqe s = {0};
  s.opcode = IORING_OP_ASYNC_CANCEL;
  s.addr = target;
  s.user_data = token;
  CHECK(cm_ring_push(&c->ring, &s) == 0 && cm_ring_submit(&c->ring) == 0);
}
static struct io_uring_cqe take(Context *c) {
  struct io_uring_cqe q;
  CHECK(cm_ring_wait(&c->ring, &q) == 0);
  CHECK(co_complete(&c->pool, q.user_data, q.res, q.flags) == 1);
  return q;
}
static void cancellation(int fixed) {
  Context c;
  CHECK(setup(&c, fixed, -1) == 0);
  int p[2];
  CHECK(pipe(p) == 0);
  uint64_t t = read_op(&c, 0, p[0], 32, fixed, 0), start = now();
  cancel_op(&c, t);
  struct io_uring_cqe a = take(&c);
  CHECK(co_release(&c.pool, t) == 0);
  struct io_uring_cqe b = take(&c);
  CHECK(c.pool.slot[0].result == -ECANCELED &&
        c.pool.slot[0].cancel_result == 0);
  CHECK(co_release(&c.pool, t) == 1 && co_quiescent(&c.pool));
  uint64_t latency = now() - start;
  uint64_t newer = read_op(&c, 0, p[0], 4, fixed, 0);
  CHECK(newer != t);
  struct io_uring_sqe s = {0};
  s.opcode = IORING_OP_ASYNC_CANCEL;
  s.addr = t;
  s.user_data = 999;
  CHECK(cm_ring_push(&c.ring, &s) == 0 && cm_ring_submit(&c.ring) == 0);
  struct io_uring_cqe q;
  CHECK(cm_ring_wait(&c.ring, &q) == 0 && q.user_data == 999 &&
        q.res == -ENOENT && q.flags == 0);
  CHECK(c.pool.slot[0].state == CO_BUSY && write(p[1], "safe", 4) == 4);
  q = take(&c);
  CHECK(q.user_data == newer && q.res == 4 && memcmp(c.map, "safe", 4) == 0);
  CHECK(co_release(&c.pool, newer) == 1);
  /* Target already completed in kernel; deliberately delay dispatch to retain
   * the application loan. Cancellation may find no request, never frees it. */
  CHECK(write(p[1], "ok", 2) == 2);
  t = read_op(&c, 1, p[0], 4, fixed, 0);
  CHECK(cm_ring_wait(&c.ring, &q) == 0 && q.res == 2);
  cancel_op(&c, t);
  CHECK(c.pool.slot[1].state == CO_BUSY &&
        co_complete(&c.pool, q.user_data, q.res, q.flags) == 1);
  CHECK(co_release(&c.pool, t) == 0);
  q = take(&c);
  CHECK(q.res == -ENOENT && co_release(&c.pool, t) == 1);
  CHECK(close(p[0]) == 0 && close(p[1]) == 0);
  cleanup(&c);
  printf("COMPLETION-CANCEL:fixed=%d;first=%s;second=%s;latency_ns=%" PRIu64
         ";retained_bytes=32;old_cancel=-ENOENT;dispatch_delayed=1;\n",
         fixed, (a.user_data >> 63) ? "cancel" : "target",
         (b.user_data >> 63) ? "cancel" : "target", latency);
}
static void boundaries(int fixed) {
  Context c;
  CHECK(setup(&c, fixed, -1) == 0);
  int p[2];
  CHECK(pipe(p) == 0);
  CHECK(write(p[1], "abc", 3) == 3);
  uint64_t t = read_op(&c, 2, p[0], 8, fixed, 17);
  struct io_uring_cqe q = take(&c);
  CHECK(q.res == 3 && memcmp(c.pool.slot[2].buffer + 17, "abc", 3) == 0 &&
        co_release(&c.pool, t) == 1);
  CHECK(close(p[1]) == 0);
  t = read_op(&c, 2, p[0], 8, fixed, 0);
  q = take(&c);
  CHECK(q.res == 0 && co_release(&c.pool, t) == 1);
  t = read_op(&c, 3, -1, 1, fixed, 0);
  q = take(&c);
  CHECK(q.res == -EBADF && co_release(&c.pool, t) == 1);
  if (fixed) {
    t = read_op(&c, 3, p[0], 2, 1, 4095);
    q = take(&c);
    CHECK(q.res == -EFAULT && co_release(&c.pool, t) == 1);
  }
  CHECK(close(p[0]) == 0);
  cleanup(&c);
  printf("COMPLETION-BOUNDARY:fixed=%d;short=3;EOF=0;bad_fd=-EBADF;subrange=17;"
         "range_rejected=%d;\n",
         fixed, fixed);
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "cancel")) {
    cancellation(0);
    cancellation(1);
  } else if (!strcmp(argv[1], "boundary")) {
    boundaries(0);
    boundaries(1);
  } else if (!strcmp(argv[1], "failure")) {
    Context c;
    for (int i = 0; i < 5; ++i)
      CHECK(setup(&c, 1, i) == -1);
    CHECK(setup(&c, 1, -1) == 0);
    cleanup(&c);
    puts("COMPLETION-FAILURE:acquisition_cuts=5;normal_cleanup=1;registered_"
         "release_before_unmap=1;");
  } else
    return 2;
}
