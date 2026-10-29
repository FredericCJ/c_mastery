#define _GNU_SOURCE
#include "posix_lifecycle.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <sys/wait.h>
#include <unistd.h>
#ifndef PL_MUTANT
#define PL_MUTANT 0
#endif
int pl_launch(const char *path, char *const argv[], unsigned fail_stage, struct pl_launch *out) {
  int channel[2], status, exec_error = 0;
  size_t used = 0;
  if (!path || !argv || !argv[0] || !out || fail_stage > 2) return EINVAL;
  if (fail_stage == 1) return EMFILE;
  if (pipe2(channel, O_CLOEXEC) < 0) return errno;
  pid_t child;
  if (fail_stage == 2) { child = -1; errno = EAGAIN; }
  else child = fork();
  if (child < 0) {
    int error = errno;
    (void)close(channel[0]); (void)close(channel[1]);
    return error;
  }
  if (child == 0) {
    (void)close(channel[0]);
    char *const environment[] = {NULL};
    execve(path, argv, environment);
    int error = errno;
    const unsigned char *bytes = (const unsigned char *)&error;
    size_t sent = 0;
    while (sent < sizeof error) {
      ssize_t count = write(channel[1], bytes + sent, sizeof error - sent);
      if (count > 0) sent += (size_t)count;
      else if (count < 0 && errno == EINTR) continue;
      else break;
    }
    _exit(127);
  }
  (void)close(channel[1]);
  int read_error = 0;
  while (used < sizeof exec_error) {
    ssize_t count = read(channel[0], (unsigned char *)&exec_error + used,
                         sizeof exec_error - used);
    if (count > 0) used += (size_t)count;
    else if (count == 0) break;
    else if (errno != EINTR) { read_error = errno; break; }
  }
  (void)close(channel[0]);
  pid_t waited;
  do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
  if (waited < 0) return errno; /* Forbidden external reaping violates this narrow contract. */
  if (read_error) return read_error;
  if (used != 0 && used != sizeof exec_error) return EPROTO;
  struct pl_launch result;
  if (used == sizeof exec_error && PL_MUTANT != 1)
    result = (struct pl_launch){PL_EXEC_ERROR, exec_error};
  else if (WIFEXITED(status))
    result = (struct pl_launch){PL_EXITED, WEXITSTATUS(status)};
  else if (WIFSIGNALED(status))
    result = (struct pl_launch){PL_SIGNALED, WTERMSIG(status)};
  else return EPROTO;
  if (PL_MUTANT == 2 && result.kind == PL_EXITED && result.value == 127)
    result = (struct pl_launch){PL_EXEC_ERROR, ENOENT};
  *out = result;
  return 0;
}
struct pt_work { unsigned input, result; };
static void *square(void *p) {
  struct pt_work *work = p;
  work->result = work->input * work->input;
  return NULL;
}
int pl_pool(unsigned workers, unsigned fail_ordinal, struct pl_pool *out) {
  pthread_t handles[4];
  struct pt_work work[4];
  struct pl_pool result = {0};
  if (!out || workers > 4 || fail_ordinal > workers) return EINVAL;
  for (unsigned i = 0; i < workers; ++i) {
    work[i] = (struct pt_work){i + 1, 0};
    int error = fail_ordinal == i + 1 && PL_MUTANT != 3 ? EAGAIN :
      pthread_create(&handles[i], NULL, square, &work[i]);
    if (error) { result.create_error = error; break; }
    ++result.created;
  }
  for (unsigned i = 0; i < result.created; ++i) {
    int error = pthread_join(handles[i], NULL);
    /* With valid handles, no cancellation and exclusive join ownership, this
       join cannot fail under the declared API preconditions. Do not return
       stack-backed arguments while a borrower could remain live. */
    if (error) _exit(125);
    ++result.joined;
    ++result.completed;
    if (PL_MUTANT != 4 || i + 1 != result.created) result.sum += work[i].result;
  }
  *out = result;
  return 0;
}
