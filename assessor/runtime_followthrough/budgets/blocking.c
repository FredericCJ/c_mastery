#define _POSIX_C_SOURCE 200809L
#include "budgets.h"
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#ifndef VL_BLOCK_SKIP_READ
#define VL_BLOCK_SKIP_READ 0
#endif
typedef struct {
  int ready, release;
} Blocking;
typedef struct {
  int status, value;
  uint32_t fuel;
  unsigned calls;
} Record;
static unsigned calls;
static int64_t milliseconds(void) {
  struct timespec t;
  if (clock_gettime(CLOCK_MONOTONIC, &t))
    return -1;
  return (int64_t)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}
static int transfer(int fd, void *buf, size_t count, int writing) {
  unsigned char *p = buf;
  while (count) {
    ssize_t n = writing ? write(fd, p, count) : read(fd, p, count);
    if (n < 0 && errno == EINTR)
      continue;
    if (n <= 0)
      return -1;
    p += n;
    count -= (size_t)n;
  }
  return 0;
}
/* Absolute deadline is a harness containment limit, not a scheduler guarantee.
 */
static int readable(int fd, int duration) {
  int64_t now = milliseconds();
  if (now < 0)
    return -1;
  int64_t end = now + duration;
  for (;;) {
    now = milliseconds();
    if (now < 0)
      return -1;
    int remain = now < end ? (int)(end - now) : 0;
    struct pollfd p = {fd, POLLIN, 0};
    int r = poll(&p, 1, remain);
    if (r < 0 && errno == EINTR)
      continue;
    if (r < 0)
      return -1;
    if (r)
      return p.revents & (POLLIN | POLLHUP) ? 1 : -1;
    if (milliseconds() >= end)
      return 0;
  }
}
/* Intentionally lacks its own deadline: this is the faulty teaching callback,
 * violating the embedding policy. Its owned parent always releases/kills it. */
static int blocking(void *ctx, uint8_t id, int32_t input, int32_t *out) {
  Blocking *b = ctx;
  char byte = 'R';
  calls++;
  if (id != 1 || transfer(b->ready, &byte, 1, 1))
    return 1;
  if (!VL_BLOCK_SKIP_READ && transfer(b->release, &byte, 1, 0))
    return 1;
  *out = input + 11;
  return 0;
}
static void close_fd(int *fd) {
  if (*fd >= 0) {
    (void)close(*fd);
    *fd = -1;
  }
}
static int reap(pid_t child, int *status) {
  int64_t end = milliseconds() + 5000;
  for (;;) {
    pid_t r = waitpid(child, status, WNOHANG);
    if (r == child)
      return 0;
    if (r < 0 && errno != EINTR)
      return -1;
    if (milliseconds() >= end)
      return -1;
    struct timespec pause = {0, 1000000};
    nanosleep(&pause, 0);
  }
}
static int experiment(unsigned mode) {
  int ready[2] = {-1, -1}, release[2] = {-1, -1}, done[2] = {-1, -1};
  pid_t child = -1;
  int result = 1, status = 0;
  char byte;
  CvmProgram p = {
      {OP_INT, 7, 0, 0, 0, OP_HOST, 1, OP_RET}, 8, 1, {{0, 8, 0, 0}}};
  Vs session = {0};
  VvWork scratch;
  VsRequest request = {0};
  CvmValue value = cvm_int(91);
  VlReport ledger;
  const char *failure = "pipe/fork setup";
  if (pipe(ready) || pipe(release) || pipe(done))
    goto cleanup;
  if (mode) {
    failure = "pending setup";
    if (vs_start(&session, &p, 0, 3, 0, 1, 2, &scratch) != VS_RUNNING ||
        vl_step(&session, &value, &ledger) != VS_RUNNING ||
        vl_step(&session, &value, &ledger) != VS_PENDING ||
        vs_take(&session, &request) != VS_PENDING)
      goto cleanup;
  }
  child = fork();
  if (child < 0)
    goto cleanup;
  if (child == 0) {
    close_fd(&ready[0]);
    close_fd(&release[1]);
    close_fd(&done[0]);
    Blocking b = {ready[1], release[0]};
    Record r = {0};
    if (!mode) {
      Cvm vm;
      cvm_init(&vm, 3, 0, 1);
      vm.host = blocking;
      vm.host_context = &b;
      CvmValue out = cvm_int(91);
      r.status = cvm_run(&vm, &p, &out);
      r.value = out.integer;
      r.fuel = vm.fuel;
      r.calls = calls;
    } else {
      int32_t out = 0;
      r.status = blocking(&b, request.service, request.argument, &out);
      r.value = out;
      r.calls = calls;
    }
    int rc = transfer(done[1], &r, sizeof r, 1);
    close_fd(&ready[1]);
    close_fd(&release[0]);
    close_fd(&done[1]);
    _exit(rc ? 3 : 0);
  }
  close_fd(&ready[1]);
  close_fd(&release[0]);
  close_fd(&done[1]);
  failure = "provider entered";
  if (readable(ready[0], 5000) != 1 || transfer(ready[0], &byte, 1, 0) ||
      byte != 'R')
    goto cleanup;
  int64_t start = milliseconds();
  failure = "no_completed_result_before_release";
  if (readable(done[0], 100) != 0 || waitpid(child, &status, WNOHANG) != 0)
    goto cleanup;
  int64_t observed = milliseconds() - start;
  if (mode) {
    /* Actual unrelated guest makes progress while first session retains
     * request. */
    CvmProgram q = {{OP_INT, 47, 0, 0, 0, OP_RET}, 6, 1, {{0, 6, 0, 0}}};
    Vs other = {0};
    CvmValue out = cvm_int(91);
    failure = "unrelated guest progress";
    if (vs_start(&other, &q, 0, 2, 0, 0, 0, &scratch) != VS_RUNNING ||
        vl_step(&other, &out, &ledger) != VS_RUNNING ||
        vl_step(&other, &out, &ledger) != VS_DONE || out.integer != 47)
      goto cleanup;
    failure = "retained pending ownership";
    if (vs_start(&session, &p, 0, 3, 0, 1, 2, &scratch) != VS_STATE ||
        vs_complete(&session, request.epoch + 1, request.serial, 0, 18) !=
            VS_STATE)
      goto cleanup;
  }
  if (mode == 2) {
    failure = "owned cancellation/quiescence";
    if (kill(child, SIGKILL) || reap(child, &status) || !WIFSIGNALED(status) ||
        WTERMSIG(status) != SIGKILL)
      goto cleanup;
    child = -1;
    if (vs_complete(&session, request.epoch, request.serial, 1, 0) !=
            CVM_HOST_ERROR ||
        vl_step(&session, &value, &ledger) != CVM_HOST_ERROR ||
        value.integer != 91)
      goto cleanup;
    printf("VL_BLOCK_PASS mode=cancel observed_wait_ms=%lld "
           "reaped_before_failure=1 value=91\n",
           (long long)observed);
    result = 0;
    goto cleanup;
  }
  byte = 'G';
  failure = "release/result";
  if (transfer(release[1], &byte, 1, 1) || readable(done[0], 5000) != 1)
    goto cleanup;
  Record r;
  if (transfer(done[0], &r, sizeof r, 0) || reap(child, &status))
    goto cleanup;
  child = -1;
  failure = "owned child result";
  if (!WIFEXITED(status) || WEXITSTATUS(status) || r.status || r.value != 18 ||
      r.calls != 1 || r.fuel)
    goto cleanup;
  if (mode) {
    failure = "pending completion";
    if (vs_complete(&session, request.epoch, request.serial, 0, r.value) !=
            VS_RUNNING ||
        vl_step(&session, &value, &ledger) != VS_DONE || value.integer != 18 ||
        session.vm.fuel)
      goto cleanup;
  }
  printf("VL_BLOCK_PASS mode=%s observed_wait_ms=%lld bounded_guest_fuel=3 "
         "value=18 reaped=1\n",
         mode ? "pending" : "blocking", (long long)observed);
  result = 0;
cleanup:
  if (child > 0) {
    (void)kill(child, SIGKILL);
    while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
    }
  }
  close_fd(&ready[0]);
  close_fd(&ready[1]);
  close_fd(&release[0]);
  close_fd(&release[1]);
  close_fd(&done[0]);
  close_fd(&done[1]);
  if (result)
    fprintf(stderr, "VL_BLOCK_REFUSE %s\n", failure);
  return result;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "blocking"))
    return experiment(0);
  if (!strcmp(argv[1], "pending"))
    return experiment(1);
  if (!strcmp(argv[1], "cancel"))
    return experiment(2);
  return 2;
}
