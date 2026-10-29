#define _GNU_SOURCE
#include "nursery.h"
#include "hosted.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifndef MFN_MUTANT
#define MFN_MUTANT 0
#endif
void mfn_cancel(struct mfn_tree *t) {
  msg_cancel(&t->outer);
  if (MFN_MUTANT != 1) {
    msg_cancel(&t->inner[0]);
    msg_cancel(&t->inner[1]);
  }
}
static int leaf_main(void *arg, const atomic_bool *cancel) {
  struct mfn_link *link = arg;
  struct mfn_tree *t = link->tree;
  unsigned i = link->index;
  int result = t->tasks[i].run(t->tasks[i].ctx, cancel);
  t->results[i] = result;
  if (result) mfn_cancel(t);
  return result;
}
static int group_main(void *arg, const atomic_bool *cancel) {
  struct mfn_link *link = arg;
  struct mfn_tree *t = link->tree;
  unsigned k = link->index;
  int first = 0;
  (void)cancel; /* Root cancellation is propagated to all descendant flags. */
  for (unsigned j = 0; j < 2; j++) {
    unsigned i = 2*k+j;
    if (msg_spawn(&t->inner[k], leaf_main, &t->leaf[i]) != MSG_OK) {
      t->group_failed[k] = true;
      mfn_cancel(t);
      break;
    }
    t->admitted[i] = true;
  }
  /* Failed joins are retried by the root AFTER this group thread is joined. */
  if (msg_join(&t->inner[k], &first) != MSG_OK) t->group_failed[k] = true;
  unsigned char done = (unsigned char)k;
  ssize_t n;
  do { n = write(t->pipefd[1], &done, 1); } while (n < 0 && errno == EINTR);
  if (n != 1) t->group_failed[k] = true;
  return first;
}
enum msg_status mfn_start(struct mfn_tree *t, const struct mfn_task tasks[4], size_t fail_outer,
                    const size_t fail_inner[2]) {
  if (!t || !tasks || !fail_inner || fail_outer > 2 ||
      fail_inner[0] > 2 || fail_inner[1] > 2) return MSG_INVALID;
  for (unsigned i = 0; i < 4; i++) if (!tasks[i].run) return MSG_INVALID;
  int descriptors[2];
  if (pipe2(descriptors, O_CLOEXEC)) return MSG_FAILED;
  /* Successful setup takes all four contexts, even if a later admission fails. */
  *t = (struct mfn_tree){.pipefd = {descriptors[0], descriptors[1]}, .active = true};
  msg_nursery_init(&t->outer, fail_outer);
  for (unsigned k = 0; k < 2; k++) {
    msg_nursery_init(&t->inner[k], fail_inner[k]);
    t->group[k] = (struct mfn_link){t, k};
  }
  for (unsigned i = 0; i < 4; i++) {
    t->tasks[i] = tasks[i];
    t->leaf[i] = (struct mfn_link){t, i};
  }
  for (unsigned k = 0; k < 2; k++) {
    if (msg_spawn(&t->outer, group_main, &t->group[k]) != MSG_OK) {
      t->start_failed = true;
      mfn_cancel(t);
      break;
    }
  }
  return MSG_OK;
}
enum msg_status mfn_wait(struct mfn_tree *t, int cancel_fd, long long deadline_ns,
                   struct mfn_outcome *out) {
  if (!t || !out || cancel_fd < 0) return MSG_INVALID;
  if (!t->active) return MSG_CLOSED;
  if (!t->waited) {
    size_t complete = 0;
    while (complete < t->outer.count) {
      unsigned char bytes[2];
      ssize_t n = hs_read_until(t->pipefd[0], bytes, 2-complete,
                                cancel_fd, deadline_ns);
      if (n <= 0) {
        t->stop = n == -2 ? MFN_DEADLINE : n == -3 ? MFN_EXTERNAL_CANCEL : MFN_IO_FAILURE;
        mfn_cancel(t);
        break;
      }
      complete += (size_t)n;
    }
    t->waited = true;
  }
  int ignored;
  if (msg_join(&t->outer, &ignored) != MSG_OK) return MSG_BUSY;
  for (unsigned k = 0; k < 2; k++)
    if (msg_join(&t->inner[k], &ignored) != MSG_OK) return MSG_BUSY;
  struct mfn_outcome result = {.stop = t->stop, .spawn_failed = t->start_failed};
  for (unsigned k = 0; k < 2; k++) result.spawn_failed |= t->group_failed[k];
  if (MFN_MUTANT == 4) result.spawn_failed = false;
  for (unsigned i = 0; i < 4; i++) {
    result.admitted[i] = t->admitted[i];
    result.errors[i] = t->results[i];
    if (result.errors[i] && (!result.primary || MFN_MUTANT == 2))
      result.primary = result.errors[i];
  }
  /* Both kinds of owned resources outlive every possible user. */
  close(t->pipefd[0]); close(t->pipefd[1]);
  for (unsigned i = 0; i < 4; i++)
    if (t->tasks[i].drop && MFN_MUTANT != 3) t->tasks[i].drop(t->tasks[i].ctx);
  t->active = false;
  *out = result;
  return MSG_OK;
}
int mfn_pool_task(void *arg, const atomic_bool *cancel) {
  struct mfn_pool *p = arg;
  if (atomic_load(cancel)) return -901;
  long sum;
  if (hs_pool_sum(p->values, p->count, p->workers, &sum)) return -902;
  if (atomic_load(cancel)) return -901;
  p->sum = sum;
  return 0;
}
