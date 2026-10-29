#include "nursery.h"
void mfn_cancel(struct mfn_tree *t) { (void)t; }
enum msg_status mfn_start(struct mfn_tree *t, const struct mfn_task a[4], size_t b, const size_t c[2]) {
  (void)t; (void)a; (void)b; (void)c; return MSG_FAILED;
}
enum msg_status mfn_wait(struct mfn_tree *t, int fd, long long deadline, struct mfn_outcome *out) {
  (void)t; (void)fd; (void)deadline; (void)out; return MSG_FAILED;
}
int mfn_pool_task(void *arg, const atomic_bool *cancel) {
  (void)arg; (void)cancel; return -1;
}
