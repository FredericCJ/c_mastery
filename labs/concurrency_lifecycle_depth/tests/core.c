#include "cl.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "CL_CHECK:%d:%s\n", __LINE__, #x);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int queue(void) {
  CLQueue q;
  CLMessage slots[2];
  unsigned char in[4] = {3, 0, 9, 7}, out[5] = {99, 99, 99, 99, 99};
  size_t n = 77;
  uint32_t p = 77, s = 77;
  cl_queue_init(&q, slots);
  CHECK(cl_recv(&q, out, 5, &n, &p, &s) == CL_EMPTY && n == 77 && out[0] == 99);
  CHECK(cl_send(&q, 1, 1, in, 5) == CL_RANGE && q.count == 0);
  CHECK(cl_send(&q, 1, 1, in, 4) == CL_OK);
  in[0] = 88;
  CHECK(cl_send(&q, 2, 1, 0, 0) == CL_OK);
  CHECK(cl_send(&q, 3, 1, in, 1) == CL_FULL && q.accepted == 2);
  CHECK(cl_recv(&q, out, 3, &n, &p, &s) == CL_RANGE && n == 77 &&
        out[0] == 99 && q.count == 2);
  CHECK(cl_recv(&q, out, 4, &n, &p, &s) == CL_OK && n == 4 && p == 1 &&
        s == 1 && out[0] == 3 && out[1] == 0 && out[3] == 7 && out[4] == 99);
  cl_close(&q);
  CHECK(cl_send(&q, 3, 1, in, 1) == CL_CLOSED);
  CHECK(cl_recv(&q, 0, 0, &n, &p, &s) == CL_OK && n == 0 && p == 2);
  CHECK(cl_recv(&q, out, 5, &n, &p, &s) == CL_CLOSED &&
        q.accepted == q.delivered + q.discarded);
  cl_queue_init(&q, slots);
  for (unsigned i = 0; i < 1000; i++) {
    in[0] = (unsigned char)i;
    CHECK(cl_send(&q, i % 3, i, in, 1) == CL_OK);
    CHECK(cl_recv(&q, out, 1, &n, &p, &s) == CL_OK && p == i % 3 && s == i &&
          out[0] == (unsigned char)i);
  }
  CHECK(cl_send(&q, 0, 1000, in, 1) == CL_OK &&
        cl_send(&q, 1, 1000, in, 1) == CL_OK);
  CHECK(cl_abort(&q) == 2 && q.discarded == 2 && q.delivered == 1000 &&
        q.accepted == 1002);
  CHECK(cl_abort(&q) == 0 && cl_recv(&q, out, 5, &n, &p, &s) == CL_CLOSED);
  cl_queue_init(&q, slots);
  q.accepted = UINT64_MAX;
  CHECK(cl_send(&q, 0, 0, in, 1) == CL_RANGE && q.count == 0);
  return 0;
}
static int pool(void) {
  CLPool p;
  CLOwnedQueue q;
  CLBuffer h[4], out = {99, 99};
  unsigned n = 99;
  unsigned char bytes[8] = {1, 2, 3, 4, 5, 6, 7, 8}, copy[8] = {0};
  cl_pool_init(&p);
  cl_owned_init(&q);
  for (unsigned i = 0; i < 4; i++)
    CHECK(cl_alloc(&p, 4, &h[i]) == CL_OK);
  CHECK(cl_release(&p, h[0]) == CL_OK && cl_release(&p, h[2]) == CL_OK);
  CHECK(cl_alloc(&p, 6, &out) == CL_FRAGMENTED && out.slot == 99 &&
        p.allocations == 4);
  CHECK(cl_release(&p, h[1]) == CL_OK && cl_alloc(&p, 6, &out) == CL_OK);
  CHECK(cl_release(&p, h[0]) == CL_STALE &&
        cl_buffer_write(&p, out, bytes, 6) == CL_OK);
  CHECK(cl_owned_send(&p, &q, out) == CL_OK);
  CHECK(cl_release(&p, out) == CL_BUSY &&
        cl_buffer_write(&p, out, bytes, 6) == CL_BUSY);
  CHECK(cl_buffer_read(&p, out, copy, 8, &n) == CL_BUSY && n == 99);
  CHECK(cl_owned_send(&p, &q, h[3]) == CL_OK);
  CHECK(cl_alloc(&p, 0, &h[0]) == CL_OK);
  CHECK(cl_owned_send(&p, &q, h[0]) == CL_FULL &&
        p.slots[h[0].slot].owner == CL_CALLER);
  /* Cancellation of an unaccepted send retains caller ownership. */
  CHECK(cl_release(&p, h[0]) == CL_OK);
  CHECK(cl_owned_recv(&p, &q, &h[1]) == CL_OK && h[1].slot == out.slot);
  CHECK(cl_buffer_read(&p, h[1], copy, 5, &n) == CL_RANGE && n == 99);
  CHECK(cl_buffer_read(&p, h[1], copy, 8, &n) == CL_OK && n == 6 &&
        copy[5] == 6);
  CHECK(cl_owned_abort(&p, &q) == 1 && cl_owned_abort(&p, &q) == 0);
  CHECK(cl_release(&p, h[1]) == CL_OK && p.allocations == p.releases);
  CHECK(cl_owned_recv(&p, &q, &out) == CL_CLOSED);
  cl_pool_init(&p);
  p.slots[0].generation = UINT64_MAX - 1;
  CHECK(cl_alloc(&p, 0, &h[0]) == CL_OK && h[0].slot == 0);
  CHECK(cl_release(&p, h[0]) == CL_OK && p.slots[0].generation == UINT64_MAX);
  CHECK(cl_alloc(&p, 1, &out) == CL_OK && out.slot == 1 &&
        cl_release(&p, h[0]) == CL_STALE);
  CHECK(cl_release(&p, out) == CL_OK);
  return 0;
}
static int life(void) {
  CLLife l;
  CLLease h[5], old;
  cl_life_init(&l);
  for (unsigned i = 0; i < 5; i++)
    CHECK(cl_acquire(&l, &h[i]) == CL_OK);
  old = h[0];
  CHECK(cl_transition(&l, h[0], CL_ACTIVE, CL_SEND_WAIT) == CL_OK);
  CHECK(cl_transition(&l, h[1], CL_ACTIVE, CL_RECV_WAIT) == CL_OK);
  CHECK(cl_transition(&l, h[2], CL_ACTIVE, CL_COND_RELOCK) == CL_OK);
  CHECK(cl_transition(&l, h[3], CL_ACTIVE, CL_TIMER) == CL_OK);
  cl_life_close(&l);
  CHECK(cl_acquire(&l, &old) == CL_CLOSED);
  CHECK(l.leases[h[0].slot].state == CL_WAKE_CLOSED &&
        l.leases[h[1].slot].state == CL_WAKE_CLOSED);
  CHECK(l.leases[h[2].slot].state == CL_COND_RELOCK &&
        l.leases[h[3].slot].state == CL_TIMER);
  CHECK(cl_life_drained(&l) == CL_OK && cl_destroy(&l) == CL_BUSY);
  CHECK(cl_drop(&l, h[0]) == CL_OK && cl_drop(&l, h[0]) == CL_STALE);
  CHECK(cl_drop(&l, h[1]) == CL_OK && cl_drop(&l, h[4]) == CL_OK);
  CHECK(cl_drop(&l, h[2]) == CL_BUSY && cl_drop(&l, h[3]) == CL_BUSY);
  CHECK(cl_transition(&l, h[2], CL_COND_RELOCK, CL_ACTIVE) == CL_OK &&
        cl_drop(&l, h[2]) == CL_OK);
  CHECK(cl_destroy(&l) == CL_BUSY);
  /* A trusted completed cancel/join, not just a cancel request, authorizes this
   * transition. */
  CHECK(cl_transition(&l, h[3], CL_TIMER, CL_ACTIVE) == CL_OK &&
        cl_drop(&l, h[3]) == CL_OK);
  CHECK(cl_destroy(&l) == CL_OK && cl_restart(&l) == CL_OK &&
        l.generation == 2);
  CHECK(cl_drop(&l, h[0]) == CL_STALE && cl_acquire(&l, &old) == CL_OK);
  CHECK(cl_transition(&l, h[0], CL_ACTIVE, CL_TIMER) == CL_STALE &&
        cl_drop(&l, old) == CL_OK);
  l.generation = UINT64_MAX;
  cl_life_close(&l);
  CHECK(cl_life_drained(&l) == CL_OK && cl_destroy(&l) == CL_OK);
  CHECK(cl_restart(&l) == CL_RANGE && l.phase == CL_DESTROYED);
  CLLife service[3];
  CLLease consumer_to_worker, worker_to_timer;
  for (unsigned i = 0; i < 3; i++)
    cl_life_init(&service[i]);
  CHECK(cl_acquire(&service[1], &consumer_to_worker) == CL_OK &&
        cl_acquire(&service[2], &worker_to_timer) == CL_OK);
  for (unsigned i = 0; i < 3; i++) {
    cl_life_close(&service[i]);
    CHECK(cl_life_drained(&service[i]) == CL_OK);
  }
  CHECK(cl_destroy(&service[2]) == CL_BUSY &&
        cl_destroy(&service[1]) == CL_BUSY);
  CHECK(cl_destroy(&service[0]) == CL_OK &&
        cl_drop(&service[1], consumer_to_worker) == CL_OK &&
        cl_destroy(&service[1]) == CL_OK);
  CHECK(cl_drop(&service[2], worker_to_timer) == CL_OK &&
        cl_destroy(&service[2]) == CL_OK);
  service[1].generation = UINT64_MAX;
  CHECK(cl_restart_group(service, 3) == CL_RANGE &&
        service[0].generation == 1 && service[2].generation == 1);
  /* A separate never-exhausted cohort tests successful reverse-order restart.
   */
  for (unsigned i = 0; i < 3; i++) {
    cl_life_init(&service[i]);
    cl_life_close(&service[i]);
    CHECK(cl_life_drained(&service[i]) == CL_OK &&
          cl_destroy(&service[i]) == CL_OK);
  }
  CHECK(cl_restart_group(service, 3) == CL_OK);
  CHECK(cl_drop(&service[1], consumer_to_worker) == CL_STALE &&
        cl_drop(&service[2], worker_to_timer) == CL_STALE);
  return 0;
}
static int record(void) {
  CLRecord r;
  uint32_t v = 99;
  cl_record_init(&r, 7);
  CHECK(cl_record_begin(&r, 1) == CL_OK && cl_record_stage(&r, 1, 13) == CL_OK);
  CHECK(cl_record_read(&r, &v) == CL_POISONED && v == 99);
  CHECK(cl_record_dead(&r, 2) == CL_BUSY && cl_record_dead(&r, 1) == CL_OK);
  CHECK(cl_record_begin(&r, 3) == CL_POISONED &&
        cl_repair_begin(&r, 2) == CL_OK);
  CHECK(cl_repair_begin(&r, 3) == CL_BUSY &&
        cl_repair_finish(&r, 3, 1) == CL_BUSY);
  CHECK(cl_repair_finish(&r, 2, 1) == CL_OK &&
        cl_record_read(&r, &v) == CL_OK && v == 7);
  CHECK(cl_record_begin(&r, 3) == CL_OK && cl_record_stage(&r, 3, 9) == CL_OK &&
        cl_record_commit(&r, 3) == CL_OK);
  CHECK(cl_record_read(&r, &v) == CL_OK && v == 9);
  CHECK(cl_record_begin(&r, 4) == CL_OK &&
        cl_record_stage(&r, 4, 20) == CL_OK && cl_record_dead(&r, 4) == CL_OK);
  CHECK(cl_repair_begin(&r, 5) == CL_OK && cl_record_dead(&r, 5) == CL_OK);
  CHECK(cl_repair_begin(&r, 6) == CL_OK && cl_record_dead(&r, 6) == CL_OK);
  CHECK(cl_repair_begin(&r, 7) == CL_UNRECOVERABLE &&
        cl_record_read(&r, &v) == CL_UNRECOVERABLE);
  cl_record_init(&r, 1);
  CHECK(cl_record_begin(&r, 1) == CL_OK && cl_record_dead(&r, 1) == CL_OK);
  CHECK(cl_repair_begin(&r, 2) == CL_OK &&
        cl_repair_finish(&r, 2, 0) == CL_UNRECOVERABLE);
  CHECK(cl_record_begin(&r, 3) == CL_UNRECOVERABLE);
  return 0;
}
static int ack(void) {
  CLAck a;
  uint32_t out = 99;
  cl_ack_init(&a, 5);
  CHECK(cl_request(&a, 5, 1, 7, &out) == CL_PENDING && out == 99);
  CHECK(cl_request(&a, 5, 1, 7, &out) == CL_PENDING &&
        cl_acknowledge(&a, 1) == CL_PENDING);
  CHECK(cl_apply(&a, 1) == CL_OK && a.value == 7);
  /* The first reply is lost; same identity and payload must retrieve the
   * receipt. */
  CHECK(cl_request(&a, 5, 1, 7, &out) == CL_OK && out == 7 && a.value == 7);
  CHECK(cl_request(&a, 5, 1, 8, &out) == CL_RANGE && a.value == 7);
  CHECK(cl_apply(&a, 1) == CL_OK && a.value == 7);
  CHECK(cl_acknowledge(&a, 1) == CL_OK &&
        cl_request(&a, 5, 1, 7, &out) == CL_STALE);
  for (unsigned i = 2; i < 6; i++)
    CHECK(cl_request(&a, 5, i, 1, &out) == CL_PENDING);
  CHECK(cl_request(&a, 5, 6, 1, &out) == CL_FULL && a.highwater == 5);
  for (unsigned i = 2; i < 6; i++)
    CHECK(cl_apply(&a, i) == CL_OK && cl_acknowledge(&a, i) == CL_OK);
  CHECK(a.value == 11 && cl_request(&a, 6, 6, 1, &out) == CL_STALE);
  CHECK(cl_request(&a, 5, UINT32_MAX, UINT32_MAX, &out) == CL_PENDING);
  CHECK(cl_apply(&a, UINT32_MAX) == CL_RANGE && a.value == 11);
  out = 99;
  CHECK(cl_request(&a, 5, UINT32_MAX, UINT32_MAX, &out) == CL_RANGE &&
        out == 99);
  CHECK(cl_acknowledge(&a, UINT32_MAX) == CL_OK &&
        cl_request(&a, 5, UINT32_MAX, UINT32_MAX, &out) == CL_STALE);
  cl_ack_init(&a, 6);
  CHECK(cl_request(&a, 5, UINT32_MAX, 1, &out) == CL_STALE && a.value == 0);
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  int r = 2;
  if (!strcmp(argv[1], "queue"))
    r = queue();
  if (!strcmp(argv[1], "pool"))
    r = pool();
  if (!strcmp(argv[1], "life"))
    r = life();
  if (!strcmp(argv[1], "record"))
    r = record();
  if (!strcmp(argv[1], "ack"))
    r = ack();
  if (!r)
    printf("CL_PASS %s\n", argv[1]);
  return r;
}
