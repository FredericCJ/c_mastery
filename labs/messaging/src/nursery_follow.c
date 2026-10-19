#include "nursery.h"
void mfn_cancel(MfnTree *t) { (void)t; }
MsgStatus mfn_start(MfnTree *t, const MfnTask a[4], size_t b, const size_t c[2]) {
  (void)t; (void)a; (void)b; (void)c; return MSG_FAILED;
}
MsgStatus mfn_wait(MfnTree *t, int fd, long long deadline, MfnOutcome *out) {
  (void)t; (void)fd; (void)deadline; (void)out; return MSG_FAILED;
}
int mfn_pool_task(void *arg, const atomic_bool *cancel) {
  (void)arg; (void)cancel; return -1;
}
