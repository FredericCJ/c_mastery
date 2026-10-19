#include "test.h"
typedef struct {
  unsigned char storage[8], received[8];
  unsigned allocations, frees, subscribed, calls, fail_alloc, fail_sub;
  uint64_t now;
  size_t received_n;
  ArResult next;
} Adapter;
static void *alloc(void *p, size_t n) {
  Adapter *a = p;
  CHECK(n <= 8);
  if (a->fail_alloc)
    return 0;
  ++a->allocations;
  return a->storage;
}
static void free_data(void *p, void *data) {
  Adapter *a = p;
  CHECK(data == a->storage && a->frees < a->allocations);
  ++a->frees;
}
static uint64_t clock_ms(void *p) { return ((Adapter *)p)->now; }
static int notify(void *p, unsigned on) {
  Adapter *a = p;
  if (on && a->fail_sub)
    return AR_FAILED;
  if (on)
    CHECK(!a->subscribed);
  else
    CHECK(a->subscribed);
  a->subscribed = on;
  return AR_OK;
}
static ArResult write_data(void *p, const unsigned char *bytes, size_t n) {
  Adapter *a = p;
  ++a->calls;
  ArResult r = a->next;
  if (r.count > 0 && (size_t)r.count <= n) {
    for (ptrdiff_t i = 0; i < r.count; ++i)
      a->received[a->received_n++] = bytes[i];
  }
  return r;
}
static ArOps ops(Adapter *a) {
  return (ArOps){a, alloc, free_data, clock_ms, write_data, notify};
}
int main(void) {
  Adapter a = {.now = 10, .next = {1, AR_PENDING}}, b = {0};
  ArTask t = {0};
  unsigned char input[] = {1, 2, 3};
  a.fail_alloc = 1;
  ArTask old = t;
  CHECK(ar_begin(&t, ops(&a), input, 3, 1, 0) == AR_FAILED &&
        !memcmp(&t, &old, sizeof t));
  a.fail_alloc = 0;
  a.fail_sub = 1;
  CHECK(ar_begin(&t, ops(&a), input, 3, 1, 0) == AR_FAILED && a.frees == 1 &&
        !a.subscribed);
  a.fail_sub = 0;
  ArOps injected = ops(&a);
  CHECK(ar_begin(&t, injected, input, 3, 1, 0) == AR_OK);
  input[0] = 99;
  injected = ops(&b);
  (void)injected;
  CHECK(ar_begin(&t, ops(&b), input, 3, 2, 0) == AR_STATE);
  CHECK(ar_poll(&t) == AR_PENDING && t.offset == 1 && a.received[0] == 1 &&
        b.calls == 0);
  a.now = 9;
  old = t;
  unsigned calls = a.calls;
  CHECK(ar_poll(&t) == AR_CONTRACT && !memcmp(&t, &old, sizeof t) &&
        a.calls == calls);
  a.now = 11;
  CHECK(ar_cancel(&t) == AR_PENDING && a.frees == 1);
  CHECK(ar_poll(&t) == AR_PENDING && a.calls == calls);
  CHECK(ar_cancel_ack(&t) == AR_CANCELLED && a.frees == 2 && !a.subscribed);
  CHECK(ar_cancel_ack(&t) == AR_STATE && ar_poll(&t) == AR_CANCELLED);
  CHECK(t.offset == 1);
  for (unsigned defect = 0; defect < 3; ++defect) {
    a.received_n = 0;
    a.next = (ArResult){defect == 0   ? 2
                        : defect == 1 ? -1
                                      : 0,
                        defect == 2 ? AR_INVALID : AR_OK};
    CHECK(ar_begin(&t, ops(&a), input, 1, 2, 0) == AR_OK);
    CHECK(ar_poll(&t) == AR_CONTRACT && !t.active && !a.subscribed);
    unsigned frees = a.frees;
    CHECK(ar_poll(&t) == AR_CONTRACT && a.frees == frees);
  }
  a.received_n = 0;
  a.next = (ArResult){1, AR_FAILED};
  CHECK(ar_begin(&t, ops(&a), input, 3, 3, 0) == AR_OK &&
        ar_poll(&t) == AR_CONTRACT && t.offset == 0);
  a.received_n = 0;
  CHECK(ar_begin(&t, ops(&a), input, 3, 3, 1) == AR_OK &&
        ar_poll(&t) == AR_FAILED && t.offset == 1);
  a.received_n = 0;
  a.next = (ArResult){3, AR_OK};
  CHECK(ar_begin(&t, ops(&a), input, 3, 4, 0) == AR_OK &&
        ar_poll(&t) == AR_OK && t.offset == 3);
  CHECK(a.allocations == a.frees);
  puts("PASS ports effects,copy,clock rollback,pinned callbacks,cancel "
       "acknowledgement,contract faults");
}
