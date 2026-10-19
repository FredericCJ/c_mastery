#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "OF_LOCALITY:%d:%s errno=%d\n", __LINE__, #x, errno);    \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static uint64_t ns(clockid_t id) {
  struct timespec t;
  if (clock_gettime(id, &t))
    abort();
  return (uint64_t)t.tv_sec * 1000000000u + (unsigned long)t.tv_nsec;
}
static uint32_t random32(uint32_t *s) {
  *s = *s * 1664525u + 1013904223u;
  return *s;
}
static unsigned long huge_kib(const void *address) {
  FILE *f = fopen("/proc/self/smaps", "r");
  if (!f)
    return ~0ul;
  char line[256];
  unsigned long low, high, answer = 0;
  unsigned selected = 0;
  while (fgets(line, sizeof line, f)) {
    if (sscanf(line, "%lx-%lx", &low, &high) == 2)
      selected = (uintptr_t)address >= low && (uintptr_t)address < high;
    else if (selected && sscanf(line, "AnonHugePages: %lu kB", &answer) == 1)
      break;
  }
  fclose(f);
  return answer;
}
__attribute__((noinline)) uint64_t of_touch(unsigned char *a, size_t pages,
                                            size_t page, const size_t *order,
                                            unsigned write, unsigned rounds) {
  uint64_t sum = 0;
  for (unsigned r = 0; r < rounds; r++)
    for (size_t i = 0; i < pages; i++) {
      size_t offset = order[i] * page;
      if (write)
        a[offset] = (unsigned char)(order[i] % 251 + 1);
      sum += a[offset];
    }
  return sum;
}
int main(int argc, char **argv) {
  unsigned competing = argc > 1 && !strcmp(argv[1], "competitor");
  long raw_page = sysconf(_SC_PAGESIZE);
  CHECK(raw_page > 0);
  size_t page = (size_t)raw_page, pages = 2048, bytes = pages * page;
  size_t *order = malloc(pages * sizeof *order);
  CHECK(order);
  int stop[2] = {-1, -1}, report[2] = {-1, -1}, file = -1;
  uint64_t competing_bytes = 0;
  pid_t child = -1;
  if (competing) {
    char name[] = "/tmp/of-reader-XXXXXX";
    file = mkstemp(name);
    CHECK(file >= 0 && !unlink(name));
    unsigned char block[4096];
    memset(block, 73, sizeof block);
    for (unsigned i = 0; i < 64; i++)
      CHECK(write(file, block, sizeof block) == sizeof block);
    CHECK(lseek(file, 0, SEEK_SET) == 0 &&
          pipe2(stop, O_NONBLOCK | O_CLOEXEC) == 0 &&
          pipe2(report, O_CLOEXEC) == 0);
    child = fork();
    CHECK(child >= 0);
    if (!child) {
      close(stop[1]);
      close(report[0]);
      uint64_t read_bytes = 0, checksum = 0;
      for (unsigned j = 0; j < 131072; j++) {
        char token;
        ssize_t control = read(stop[0], &token, 1);
        if (control == 0)
          break;
        ssize_t got = read(file, block, sizeof block);
        if (got == 0) {
          if (lseek(file, 0, SEEK_SET) != 0)
            _exit(4);
          continue;
        }
        if (got < 0)
          _exit(5);
        read_bytes += (size_t)got;
        for (ssize_t i = 0; i < got; i++)
          checksum += block[i];
      }
      if (checksum != read_bytes * 73 ||
          write(report[1], &read_bytes, sizeof read_bytes) != sizeof read_bytes)
        _exit(6);
      close(file);
      close(report[1]);
      close(stop[0]);
      _exit(0);
    }
    CHECK(!close(file) && !close(report[1]) && !close(stop[0]));
  }
  uint32_t rng = 0x49cafe;
  printf("{\"mapping\":\"MAP_PRIVATE|MAP_ANONYMOUS\",\"page_size\":%zu,"
         "\"bytes\":%zu,\"seed\":%u,\"competitor\":%u,\"rows\":[",
         page, bytes, rng, competing);
  unsigned comma = 0;
  for (unsigned trial = 0; trial < 9; trial++) {
    unsigned phase[3] = {0, 1, 2};
    for (unsigned j = 2; j > 0; j--) {
      unsigned k = random32(&rng) % (j + 1), tmp = phase[j];
      phase[j] = phase[k];
      phase[k] = tmp;
    }
    for (unsigned index = 0; index < 3; index++)
      for (unsigned write = 0; write < 2; write++) {
        unsigned mode = phase[index];
        for (size_t i = 0; i < pages; i++)
          order[i] = mode == 1 ? (i * 17) % pages : i;
        if (mode == 2)
          for (size_t i = pages - 1; i > 0; i--) {
            size_t j = random32(&rng) % (i + 1), tmp = order[i];
            order[i] = order[j];
            order[j] = tmp;
          }
        uint64_t allocation_start = ns(CLOCK_MONOTONIC);
        unsigned char *a = mmap(NULL, bytes, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        CHECK(a != MAP_FAILED);
        uint64_t allocation_end = ns(CLOCK_MONOTONIC);
        struct rusage before, after;
        CHECK(getrusage(RUSAGE_SELF, &before) == 0);
        uint64_t start = ns(CLOCK_MONOTONIC),
                 cpu = ns(CLOCK_PROCESS_CPUTIME_ID),
                 sum = of_touch(a, pages, page, order, write, 16);
        cpu = ns(CLOCK_PROCESS_CPUTIME_ID) - cpu;
        uint64_t wall = ns(CLOCK_MONOTONIC) - start;
        CHECK(getrusage(RUSAGE_SELF, &after) == 0);
        uint64_t expected = 0;
        for (size_t i = 0; i < pages; i++)
          expected += write ? (i % 251 + 1) * 16 : 0;
        CHECK(sum == expected);
        printf("%s{\"trial\":%u,\"position\":%u,\"pattern\":%u,\"write_first\":"
               "%u,\"checksum\":%" PRIu64 ",\"allocation_start_ns\":%" PRIu64
               ",\"allocation_end_ns\":%" PRIu64 ",\"touch_start_ns\":%" PRIu64
               ",\"wall_ns\":%" PRIu64 ",\"cpu_ns\":%" PRIu64
               ",\"minor\":%ld,\"major\":%ld,\"maxrss_kib\":%ld,\"anon_huge_"
               "kib\":%lu}",
               comma++ ? "," : "", trial, index, mode, write, sum,
               allocation_start, allocation_end, start, wall, cpu,
               after.ru_minflt - before.ru_minflt,
               after.ru_majflt - before.ru_majflt, after.ru_maxrss,
               huge_kib(a));
        CHECK(munmap(a, bytes) == 0);
      }
  }
  free(order);
  if (competing) {
    CHECK(close(stop[1]) == 0 &&
          read(report[0], &competing_bytes, sizeof competing_bytes) ==
              sizeof competing_bytes &&
          !close(report[0]));
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          !WEXITSTATUS(status));
  }
  printf("],\"competing_reader_bytes\":%" PRIu64 ",\"passed\":true}\n",
         competing_bytes);
  return 0;
}
