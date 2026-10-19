#include "test.h"
typedef struct {
  unsigned a, b, dropped, calls;
  int fail_a, fail_b;
} Recovery;
static int log_full(void *p, unsigned e) {
  Recovery *r = p;
  (void)e;
  ++r->calls;
  return AR_FULL;
}
static int init_a(void *p) {
  Recovery *r = p;
  if (r->fail_a)
    return AR_FAILED;
  ++r->a;
  return AR_OK;
}
static int init_b(void *p) {
  Recovery *r = p;
  if (r->fail_b)
    return AR_FAILED;
  ++r->b;
  return AR_OK;
}
static void drop_a(void *p) {
  Recovery *r = p;
  CHECK(r->a);
  --r->a;
  ++r->dropped;
}
int main(void) {
  ArSupervisor s = {0};
  Recovery r = {0};
  unsigned events[] = {0, 1, 1, 0, 2}, modes[] = {0, 0, 1, 0, 2};
  for (unsigned i = 0; i < 5; ++i)
    CHECK(ar_supervise(&s, events[i], log_full, &r) == AR_OK &&
          s.mode == modes[i] && s.lost_logs == i + 1);
  CHECK(s.streak == 0 && r.calls == 5);
  CHECK(ar_supervise(&s, 0, log_full, &r) == AR_STATE && r.calls == 5);
  r.fail_b = 1;
  ArRecovery ports = {&r, init_a, init_b, drop_a};
  CHECK(ar_recover(&s, ports) == AR_FAILED && r.a == 0 && r.b == 0 &&
        r.dropped == 1 && s.mode == 2);
  r.fail_b = 0;
  CHECK(ar_recover(&s, ports) == AR_OK && r.a == 1 && r.b == 1 && s.mode == 0);
  CHECK(ar_recover(&s, ports) == AR_STATE);
  ArSupervisor fresh = {.mode = 2};
  Recovery other = {0};
  CHECK(ar_recover(&fresh, (ArRecovery){&other, init_a, init_b, drop_a}) ==
        AR_OK);
  CHECK(fresh.attempts == 1 && other.a == 1 && other.b == 1);
  CHECK(ar_recover(&fresh, (ArRecovery){&other, init_a, init_b, drop_a}) ==
        AR_STATE);
  CHECK(other.a == 1 && other.b == 1 && fresh.attempts == 1);
  puts("PASS supervisor primary cause,logger exhaustion,dependent "
       "rollback,bounded restart");
}
