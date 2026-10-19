#define _GNU_SOURCE
#include "completion.h"
#include "raw_ring.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "WORKLOAD-CHECK:%d:%s errno=%d\n", __LINE__, #x, errno); \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
enum { FILE_BYTES = 1048576, REQUESTS = 64, CAPACITY = 4096 };
static uint64_t clock_ns(clockid_t clock) {
  struct timespec t;
  CHECK(clock_gettime(clock, &t) == 0);
  return (uint64_t)t.tv_sec * 1000000000u + (unsigned long)t.tv_nsec;
}
static unsigned char pattern(size_t offset) {
  return (unsigned char)((offset * 17 + offset / 251) % 251);
}
static size_t offset_for(unsigned id, size_t length) {
  size_t offset =
      length == 64 ? (size_t)((id * 1619u) % 16384u) * 64 : (size_t)id * 4096;
#ifdef CO_WORKLOAD_MUTANT_OFFSET
  ++offset;
#endif
  return offset;
}
static void delay_until(uint64_t deadline) {
  for (;;) {
    uint64_t n = clock_ns(CLOCK_MONOTONIC);
    if (n >= deadline)
      return;
    uint64_t d = deadline - n;
    struct timespec t = {(time_t)(d / 1000000000u), (long)(d % 1000000000u)};
    if (nanosleep(&t, NULL) == 0)
      return;
    CHECK(errno == EINTR);
  }
}
static int create_file(const char *directory) {
  char name[4096];
  CHECK(snprintf(name, sizeof name, "%s/cmastery-io-XXXXXX", directory) > 0);
  int fd = mkstemp(name);
  CHECK(fd >= 0 && unlink(name) == 0);
  unsigned char b[4096];
  for (size_t off = 0; off < FILE_BYTES; off += sizeof b) {
    for (size_t i = 0; i < sizeof b; ++i)
      b[i] = pattern(off + i);
    CHECK(write(fd, b, sizeof b) == sizeof b);
  }
  CHECK(fsync(fd) == 0);
  return fd;
}
static unsigned resident(int fd, int cold) {
  if (cold)
    CHECK(posix_fadvise(fd, 0, FILE_BYTES, POSIX_FADV_DONTNEED) == 0);
  else {
    unsigned char data[4096];
    for (off_t off = 0; off < FILE_BYTES; off += 4096)
      CHECK(pread(fd, data, sizeof data, off) == sizeof data);
  }
  long page = sysconf(_SC_PAGESIZE);
  CHECK(page > 0 && FILE_BYTES % (unsigned long)page == 0);
  size_t count = FILE_BYTES / (size_t)page;
  unsigned char *v = calloc(count, 1);
  CHECK(v);
  void *map = mmap(NULL, FILE_BYTES, PROT_READ, MAP_SHARED, fd, 0);
  CHECK(map != MAP_FAILED && mincore(map, FILE_BYTES, v) == 0);
  unsigned sum = 0;
  for (size_t i = 0; i < count; ++i)
    sum += (v[i] & 1u) != 0;
  CHECK(munmap(map, FILE_BYTES) == 0);
  free(v);
  return sum;
}
int main(int argc, char **argv) {
  CHECK(argc == 7);
  const char *backend = argv[1];
  unsigned depth = (unsigned)strtoul(argv[2], NULL, 10);
  size_t length = (size_t)strtoul(argv[3], NULL, 10);
  int cold = !strcmp(argv[4], "cold");
  unsigned trial = (unsigned)strtoul(argv[5], NULL, 10);
  CHECK((depth == 1 || depth == 4) && (length == 64 || length == 4096));
  int ring_mode = !strcmp(backend, "ring") || !strcmp(backend, "fixed"),
      fixed = !strcmp(backend, "fixed"),
      send_mode = !strcmp(backend, "sendfile");
  CHECK(ring_mode || send_mode || !strcmp(backend, "readiness"));
  int fd = create_file(argv[6]);
  struct statfs fs;
  CHECK(fstatfs(fd, &fs) == 0);
  unsigned pages = resident(fd, cold);
  uint64_t start = clock_ns(CLOCK_MONOTONIC),
           cpu_start = clock_ns(CLOCK_PROCESS_CPUTIME_ID), pin_start = 0,
           pin_ns = 0;
  CMRawRing ring = {.fd = -1};
  unsigned char *map = NULL;
  CoPool pool;
  unsigned char *buffers[4] = {0};
  if (!send_mode) {
    map = mmap(NULL, 16384, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
               -1, 0);
    CHECK(map != MAP_FAILED);
    for (unsigned i = 0; i < 4; ++i)
      buffers[i] = map + 4096 * i;
    CHECK(co_init(&pool, buffers, 4096, 0x7fffffffu) == 0);
  }
  if (ring_mode) {
    int rc = cm_ring_init(&ring, 8);
    if (rc) {
      fprintf(stderr, "WORKLOAD-UNAVAILABLE:ring=%d\n", rc);
      return 77;
    }
  }
  if (fixed) {
    struct iovec v[4];
    for (unsigned i = 0; i < 4; ++i)
      v[i] = (struct iovec){buffers[i], 4096};
    int rc = (int)syscall(__NR_io_uring_register, ring.fd,
                          IORING_REGISTER_BUFFERS, v, 4);
    if (rc) {
      fprintf(stderr, "WORKLOAD-UNAVAILABLE:register=%d\n", errno);
      return 77;
    }
    pin_start = clock_ns(CLOCK_MONOTONIC);
  }
  int sockets[4][2];
  for (unsigned i = 0; i < depth; ++i) {
    CHECK(socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                     sockets[i]) == 0);
    int size = 1024;
    CHECK(setsockopt(sockets[i][0], SOL_SOCKET, SO_SNDBUF, &size,
                     sizeof size) == 0);
  }
  uint64_t setup_end = clock_ns(CLOCK_MONOTONIC), latencies[REQUESTS];
  size_t maximum_buffer = 0, maximum_unconsumed = 0, total = 0;
  unsigned would_block = 0, partial = 0, syscalls = 0;
  uint64_t digest = 0;
  for (unsigned base = 0; base < REQUESTS; base += depth) {
    size_t sent[4] = {0}, received[4] = {0};
    uint64_t token[4] = {0}, admitted[4] = {0};
    for (unsigned i = 0; i < depth; ++i) {
      unsigned id = base + i;
      admitted[i] = clock_ns(CLOCK_MONOTONIC);
      if (ring_mode) {
        CHECK(co_acquire(&pool, i, length, &token[i]) == 1);
        struct io_uring_sqe s = {0};
        s.opcode = fixed ? IORING_OP_READ_FIXED : IORING_OP_READ;
        s.fd = fd;
        s.addr = (uintptr_t)buffers[i];
        s.len = (unsigned)length;
        s.off = offset_for(id, length);
        s.buf_index = (unsigned short)i;
        s.user_data = token[i];
        CHECK(cm_ring_push(&ring, &s) == 0);
      } else if (!send_mode) {
        ssize_t n =
            pread(fd, buffers[i], length, (off_t)offset_for(id, length));
        ++syscalls;
        CHECK(n == (ssize_t)length);
      }
    }
    if (ring_mode) {
      CHECK(cm_ring_submit(&ring) == 0);
      ++syscalls;
      for (unsigned i = 0; i < depth; ++i) {
        struct io_uring_cqe q;
        CHECK(cm_ring_wait(&ring, &q) == 0 &&
              co_complete(&pool, q.user_data, q.res, q.flags) == 1 &&
              q.res == (int)length);
        ++syscalls;
      }
    }
    size_t retained = send_mode ? 0 : depth * length;
    if (retained > maximum_buffer)
      maximum_buffer = retained;
    unsigned complete = 0;
    uint64_t stalled_until = base == 0 ? setup_end + 10000000u : 0;
    while (complete < depth) {
      int progress = 0;
      for (unsigned i = 0; i < depth; ++i)
        if (sent[i] < length) {
          ssize_t n;
          size_t wanted = length - sent[i];
          if (send_mode) {
            off_t position = (off_t)(offset_for(base + i, length) + sent[i]);
            n = sendfile(sockets[i][0], fd, &position, wanted);
            ++syscalls;
            if (n >= 0)
              CHECK(position == (off_t)(offset_for(base + i, length) + sent[i] +
                                        (size_t)n));
          } else {
            n = write(sockets[i][0], buffers[i] + sent[i], wanted);
            ++syscalls;
          }
          if (n > 0) {
            if ((size_t)n < wanted)
              ++partial;
            sent[i] += (size_t)n;
            progress = 1;
          } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            ++would_block;
          else if (n < 0 && errno == EINTR) {
          } else
            CHECK(0);
        }
      size_t outstanding = 0;
      for (unsigned i = 0; i < depth; ++i)
        outstanding += sent[i] - received[i];
      if (outstanding > maximum_unconsumed)
        maximum_unconsumed = outstanding;
      if (stalled_until) {
        delay_until(stalled_until);
        stalled_until = 0;
      }
      for (unsigned i = 0; i < depth; ++i)
        if (received[i] < length) {
          unsigned char data[1024];
          size_t wanted = length - received[i];
          if (wanted > sizeof data)
            wanted = sizeof data;
          ssize_t n = read(sockets[i][1], data, wanted);
          ++syscalls;
          if (n > 0) {
            for (ssize_t j = 0; j < n; ++j) {
              CHECK(data[j] == pattern(offset_for(base + i, length) +
                                       received[i] + (size_t)j));
              digest += data[j];
            }
            received[i] += (size_t)n;
            progress = 1;
            if (received[i] == length) {
              latencies[base + i] = clock_ns(CLOCK_MONOTONIC) - admitted[i];
              ++complete;
              total += length;
              if (ring_mode)
                CHECK(co_release(&pool, token[i]) == 1);
            }
          } else
            CHECK(n < 0 &&
                  (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR));
        }
      if (!progress) {
        struct pollfd p[8];
        for (unsigned i = 0; i < depth; ++i) {
          p[2 * i] =
              (struct pollfd){sockets[i][0], sent[i] < length ? POLLOUT : 0, 0};
          p[2 * i + 1] = (struct pollfd){sockets[i][1],
                                         received[i] < length ? POLLIN : 0, 0};
        }
        int n = poll(p, 2 * depth, 2000);
        ++syscalls;
        CHECK(n > 0 || (n < 0 && errno == EINTR));
      }
    }
  }
  CHECK(total == REQUESTS * length && (send_mode || co_quiescent(&pool)));
  uint64_t workload_end = clock_ns(CLOCK_MONOTONIC);
  for (unsigned i = 0; i < depth; ++i)
    CHECK(close(sockets[i][0]) == 0 && close(sockets[i][1]) == 0);
  if (fixed) {
    CHECK(syscall(__NR_io_uring_register, ring.fd, IORING_UNREGISTER_BUFFERS,
                  NULL, 0) == 0);
    pin_ns = clock_ns(CLOCK_MONOTONIC) - pin_start;
  }
  size_t ring_bytes =
      ring_mode ? ring.sq_bytes + (!ring.shared_mapping ? ring.cq_bytes : 0) +
                      ring.entries_bytes
                : 0;
  if (ring_mode) {
    cm_ring_close(&ring);
  }
  if (map) {
    CHECK(munmap(map, 16384) == 0);
  }
  CHECK(close(fd) == 0);
  uint64_t elapsed = clock_ns(CLOCK_MONOTONIC) - start,
           cpu = clock_ns(CLOCK_PROCESS_CPUTIME_ID) - cpu_start;
  printf(
      "{\"backend\":\"%s\",\"depth\":%u,\"length\":%zu,\"cache_request\":\"%"
      "s\",\"resident_pages_before\":%u,\"page_size\":%ld,\"filesystem_type\":%"
      "ld,\"trial\":%u,\"requests\":64,\"bytes\":%zu,\"digest\":%" PRIu64
      ",\"whole_ns\":%" PRIu64 ",\"setup_ns\":%" PRIu64
      ",\"workload_ns\":%" PRIu64 ",\"cpu_ns\":%" PRIu64
      ",\"pin_interval_ns\":%" PRIu64
      ",\"registered_payload_bytes\":%u,\"allocated_payload_bytes\":%u,"
      "\"maximum_retained_response_bytes\":%zu,\"maximum_sent_unconsumed_"
      "bytes\":%zu,\"ring_mapping_bytes\":%zu,\"file_immutable_bytes\":1048576,"
      "\"would_block\":%u,\"partial\":%u,\"api_calls\":%u,\"latency_ns\":[",
      backend, depth, length, cold ? "cold" : "warm", pages,
      sysconf(_SC_PAGESIZE), (long)fs.f_type, trial, total, digest, elapsed,
      setup_end - start, workload_end - setup_end, cpu, pin_ns,
      fixed ? 16384u : 0u, send_mode ? 0u : 16384u, maximum_buffer,
      maximum_unconsumed, ring_bytes, would_block, partial, syscalls);
  for (unsigned i = 0; i < REQUESTS; ++i) {
    printf("%s%" PRIu64, i ? "," : "", latencies[i]);
  }
  puts("]}");
}
