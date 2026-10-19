#define _GNU_SOURCE
#include "processlab.h"
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "PROCESSLAB-CHECK:%s line%d\n", #x, __LINE__);           \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static pthread_mutex_t environment = PTHREAD_MUTEX_INITIALIZER;
static unsigned env_errors;
static void *mutate(void *unused) {
  (void)unused;
  for (unsigned i = 0; i < 1000; ++i) {
    if (pthread_mutex_lock(&environment))
      abort();
    if (setenv("CM_LIBRARY_MUTABLE", i % 2 ? "alpha" : "beta", 1))
      ++env_errors;
    if (pthread_mutex_unlock(&environment))
      abort();
  }
  return NULL;
}
static void *snapshot(void *unused) {
  (void)unused;
  for (unsigned i = 0; i < 1000; ++i) {
    char out[64];
    if (pthread_mutex_lock(&environment))
      abort();
    if (lt_environment("CM_LIBRARY_MUTABLE", out) != LT_OK ||
        (strcmp(out, "alpha") && strcmp(out, "beta")))
      ++env_errors;
    if (pthread_mutex_unlock(&environment))
      abort();
  }
  return NULL;
}
struct worker {
  uint32_t checksum;
  unsigned iterations;
};
static void *busy(void *ptr) {
  struct worker *w = ptr;
  volatile uint32_t value = 17;
  for (unsigned i = 0; i < w->iterations; ++i)
    value = value * 1664525u + 1013904223u;
  w->checksum = value;
  return NULL;
}
static long double seconds(struct timespec t) {
  return (long double)t.tv_sec + (long double)t.tv_nsec / 1000000000.L;
}
static int measure(const char *label, int mode) {
  struct timespec a, b;
  clock_t ca = clock();
  if (clock_gettime(CLOCK_MONOTONIC, &a))
    return 1;
  struct worker workers[4] = {
      {0, 20000000}, {0, 20000000}, {0, 20000000}, {0, 20000000}};
  pthread_t threads[4];
  if (mode == 3) {
    workers[0].iterations = 100000000;
    busy(workers);
  } else if (mode == 0)
    busy(workers);
  else if (mode == 1) {
    struct timespec wait = {0, 30000000};
    while (nanosleep(&wait, &wait))
      if (errno != EINTR)
        return 1;
  } else {
    for (unsigned i = 0; i < 4; ++i)
      if (pthread_create(threads + i, NULL, busy, workers + i))
        abort();
    for (unsigned i = 0; i < 4; ++i)
      if (pthread_join(threads[i], NULL))
        abort();
  }
  if (clock_gettime(CLOCK_MONOTONIC, &b))
    return 1;
  clock_t cb = clock();
  long double cpu;
  if (lt_cpu_interval(ca, cb, &cpu) != LT_OK)
    return 1;
  long double elapsed = seconds(b) - seconds(a);
  if (elapsed < 0)
    return 1;
  printf("sample=%s cpu_seconds=%.9Lf elapsed_seconds=%.9Lf checksum=%u;no "
         "ratio guarantee\n",
         label, cpu, elapsed, workers[0].checksum);
  return 0;
}
int main(int argc, char **argv) {
  (void)argv;
  CHECK(setenv("CM_LIBRARY_MUTABLE", "alpha", 1) == 0);
  pthread_t a, b;
  CHECK(pthread_create(&a, NULL, mutate, NULL) == 0);
  CHECK(pthread_create(&b, NULL, snapshot, NULL) == 0);
  CHECK(pthread_join(a, NULL) == 0 && pthread_join(b, NULL) == 0 &&
        env_errors == 0);
  CHECK(pthread_mutex_destroy(&environment) == 0);
  if (argc > 1) {
    puts("PROCESSLAB-PASS:1000 serialized changes and1000 owned snapshots");
    return 0;
  }
  struct timespec resolution;
  CHECK(clock_getres(CLOCK_MONOTONIC, &resolution) == 0);
  printf("monotonic_resolution=%ld.%09ld;clock_scale=%.0Lf\n",
         (long)resolution.tv_sec, resolution.tv_nsec,
         (long double)CLOCKS_PER_SEC);
  CHECK(measure("warmup", 0) == 0);
  for (unsigned i = 0; i < 3; ++i) {
    CHECK(measure("cpu", 0) == 0);
    CHECK(measure("wait", 1) == 0);
    CHECK(measure("four-threads", 2) == 0);
  }
  /* Constrain only this disposable process/its later threads; no host setting.
   */
  cpu_set_t allowed, one;
  CHECK(sched_getaffinity(0, sizeof allowed, &allowed) == 0);
  int cpu = -1;
  for (int i = 0; i < CPU_SETSIZE; ++i)
    if (CPU_ISSET(i, &allowed)) {
      cpu = i;
      break;
    }
  CHECK(cpu >= 0);
  CPU_ZERO(&one);
  CPU_SET(cpu, &one);
  CHECK(sched_setaffinity(0, sizeof one, &one) == 0);
  CHECK(measure("four-threads-one-cpu", 2) == 0);
  CHECK(sched_setaffinity(0, sizeof allowed, &allowed) == 0);
  CHECK(sched_setaffinity(0, sizeof one, &one) == 0);
  pid_t competitors[3];
  int ready[2];
  CHECK(pipe(ready) == 0);
  for (unsigned i = 0; i < 3; ++i) {
    competitors[i] = fork();
    CHECK(competitors[i] >= 0);
    if (!competitors[i]) {
      close(ready[0]);
      if (write(ready[1], "R", 1) != 1)
        _Exit(2);
      close(ready[1]);
      struct worker own = {0, 500000000};
      busy(&own);
      _Exit(0);
    }
  }
  CHECK(close(ready[1]) == 0);
  for (unsigned i = 0; i < 3; ++i) {
    char byte;
    ssize_t n;
    do {
      n = read(ready[0], &byte, 1);
    } while (n < 0 && errno == EINTR);
    CHECK(n == 1 && byte == 'R');
  }
  CHECK(close(ready[0]) == 0);
  CHECK(measure("three-competing-processes-one-cpu", 3) == 0);
  for (unsigned i = 0; i < 3; ++i) {
    int status;
    CHECK(waitpid(competitors[i], &status, 0) == competitors[i] &&
          WIFEXITED(status) && WEXITSTATUS(status) == 0);
  }
  CHECK(sched_setaffinity(0, sizeof allowed, &allowed) == 0);
  puts("PROCESSLAB-PASS:1000 serialized changes/snapshots;12 timing samples "
       "including warmup and constrained CPU");
  return 0;
}
