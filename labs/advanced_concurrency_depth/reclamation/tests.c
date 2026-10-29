#define _POSIX_C_SOURCE 200809L
#include "reclamation.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "AP_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x);         \
      exit(41);                                                                \
    }                                                                          \
  } while (0)
static struct ap_value value(unsigned n) {
  struct ap_value v = {n, ~n};
  return v;
}
static _Thread_local unsigned role;
struct ag_gate {
  pthread_mutex_t lock;
  pthread_cond_t cv;
  unsigned point, node, armed, reached, released, first_validated;
};
static void gate_init(struct ag_gate *g, unsigned point, unsigned node) {
  CHECK(!pthread_mutex_init(&g->lock, NULL));
  CHECK(!pthread_cond_init(&g->cv, NULL));
  g->point = point;
  g->node = node;
  g->armed = 1;
  g->reached = g->released = 0;
  g->first_validated = AP_NONE;
}
static void hook(void *ctx, unsigned point, unsigned p, unsigned node) {
  (void)p;
  struct ag_gate *g = ctx;
  if (role != 1)
    return;
  CHECK(!pthread_mutex_lock(&g->lock));
  if (point == AP_VALIDATED && g->first_validated == AP_NONE)
    g->first_validated = node;
  if (g->armed && point == g->point && node == g->node) {
    g->armed = 0;
    g->reached = 1;
    CHECK(!pthread_cond_broadcast(&g->cv));
    while (!g->released)
      CHECK(!pthread_cond_wait(&g->cv, &g->lock));
  }
  CHECK(!pthread_mutex_unlock(&g->lock));
}
static void reached(struct ag_gate *g) {
  CHECK(!pthread_mutex_lock(&g->lock));
  while (!g->reached)
    CHECK(!pthread_cond_wait(&g->cv, &g->lock));
  CHECK(!pthread_mutex_unlock(&g->lock));
}
static void release_gate(struct ag_gate *g) {
  CHECK(!pthread_mutex_lock(&g->lock));
  g->released = 1;
  CHECK(!pthread_cond_broadcast(&g->cv));
  CHECK(!pthread_mutex_unlock(&g->lock));
}
static void gate_destroy(struct ag_gate *g) {
  CHECK(!pthread_cond_destroy(&g->cv));
  CHECK(!pthread_mutex_destroy(&g->lock));
}
struct ag_operation {
  struct ap_queue *q;
  struct ap_lease lease;
  unsigned operation;
  int result;
  struct ap_value out;
};
static void *operate(void *ptr) {
  struct ag_operation *op = ptr;
  role = 1;
  if (op->operation == 0)
    op->result = ap_pop(op->q, op->lease, &op->out);
  else if (op->operation == 1)
    op->result = ap_push(op->q, op->lease, value(11));
  else
    op->result = (int)ap_collect(op->q);
  return NULL;
}
static void setup(struct ap_queue *q, struct ag_gate *g, struct ap_lease *reader, struct ap_lease *actor) {
  CHECK(ap_init(q, 4, UINT_MAX >> 2, hook, g) == AP_OK);
  CHECK(ap_register(q, reader) == AP_OK);
  CHECK(ap_register(q, actor) == AP_OK);
}
static void validation(void) {
  struct ag_gate g;
  gate_init(&g, AP_LOADED, 0);
  struct ap_queue q;
  struct ap_lease reader, actor;
  setup(&q, &g, &reader, &actor);
  CHECK(ap_push(&q, actor, value(11)) == AP_OK);
  struct ag_operation op = {&q, reader, 0, AP_UNIMPLEMENTED, {0, 0}};
  pthread_t thread;
  CHECK(!pthread_create(&thread, NULL, operate, &op));
  reached(&g);
  struct ap_value v;
  CHECK(ap_pop(&q, actor, &v) == AP_OK && v.sequence == 11);
  CHECK(ap_collect(&q) == 1);
  CHECK(ap_push(&q, actor, value(22)) == AP_OK);
  release_gate(&g);
  CHECK(!pthread_join(thread, NULL));
  CHECK(g.first_validated == 1);
  CHECK(op.result == AP_OK && op.out.sequence == 22 && op.out.inverse == ~22u);
  gate_destroy(&g);
  puts("OBS validation loaded-old-before-protection "
       "retry-before-hazardous-access");
}
static void protection(void) {
  struct ag_gate g;
  gate_init(&g, AP_VALIDATED, 0);
  struct ap_queue q;
  struct ap_lease reader, actor;
  setup(&q, &g, &reader, &actor);
  CHECK(ap_push(&q, actor, value(11)) == AP_OK);
  CHECK(ap_push(&q, actor, value(22)) == AP_OK);
  struct ag_operation op = {&q, reader, 0, AP_UNIMPLEMENTED, {0, 0}};
  pthread_t thread;
  CHECK(!pthread_create(&thread, NULL, operate, &op));
  reached(&g);
  struct ap_value v;
  CHECK(ap_pop(&q, actor, &v) == AP_OK && v.sequence == 11);
  unsigned collected = ap_collect(&q), state = atomic_load(&q.node[0].state);
  release_gate(&g);
  CHECK(!pthread_join(thread, NULL));
  CHECK(collected == 0 && state == 2);
  CHECK(op.result == AP_OK && op.out.sequence == 22);
  CHECK(ap_collect(&q) == 2);
  gate_destroy(&g);
  puts("OBS protection live-hazard-retains-old-dummy "
       "normal-clear-permits-collect");
}
static void publication(void) {
  struct ag_gate g;
  gate_init(&g, AP_LINKED, 1);
  struct ap_queue q;
  struct ap_lease writer, actor;
  setup(&q, &g, &writer, &actor);
  struct ag_operation op = {&q, writer, 1, AP_UNIMPLEMENTED, {0, 0}};
  pthread_t thread;
  CHECK(!pthread_create(&thread, NULL, operate, &op));
  reached(&g);
  unsigned lagging = atomic_load(&q.tail);
  struct ap_value v;
  CHECK(ap_pop(&q, actor, &v) == AP_OK && v.sequence == 11);
  unsigned after = atomic_load(&q.tail);
  release_gate(&g);
  CHECK(!pthread_join(thread, NULL));
  CHECK(lagging == 0 && after == 1 && op.result == AP_OK);
  gate_destroy(&g);
  puts("OBS publication actual-paused-publisher consumer-helps-tail");
}
static void collector(void) {
  struct ag_gate g;
  gate_init(&g, AP_RETIRE_SNAPSHOT, 0);
  struct ap_queue q;
  struct ap_lease reader, actor;
  setup(&q, &g, &reader, &actor);
  struct ap_value v;
  CHECK(ap_push(&q, actor, value(11)) == AP_OK);
  CHECK(ap_pop(&q, actor, &v) == AP_OK);
  struct ag_operation op = {&q, reader, 2, AP_UNIMPLEMENTED, {0, 0}};
  pthread_t thread;
  CHECK(!pthread_create(&thread, NULL, operate, &op));
  reached(&g);
  CHECK(ap_collect(&q) == 1);
  CHECK(ap_push(&q, actor, value(22)) == AP_OK);
  CHECK(ap_pop(&q, actor, &v) == AP_OK);
  CHECK(ap_push(&q, actor, value(33)) == AP_OK);
  CHECK(ap_pop(&q, actor, &v) == AP_OK);
  CHECK(atomic_load(&q.node[0].state) == 6);
  release_gate(&g);
  CHECK(!pthread_join(thread, NULL));
  CHECK(atomic_load(&q.node[0].state) == 6);
  gate_destroy(&g);
  puts("OBS collector "
       "stale-retirement-generation-cannot-free-later-incarnation");
}
static void generation(void) {
  for (unsigned limit = 1; limit <= 3; limit += 2) {
    struct ap_queue q;
    struct ap_lease lease;
    struct ap_value v;
    CHECK(ap_init(&q, 1, limit, NULL, NULL) == AP_OK);
    CHECK(ap_register(&q, &lease) == AP_OK);
    unsigned expected = 32 * (limit + 1) - 1, count = 0;
    while (ap_push(&q, lease, value(count)) == AP_OK) {
      CHECK(count < expected);
      CHECK(ap_pop(&q, lease, &v) == AP_OK && v.sequence == count);
      ap_collect(&q);
      count++;
    }
    CHECK(count == expected);
    unsigned retired = 0;
    for (unsigned i = 0; i < 32; i++)
      retired += ((atomic_load(&q.node[i].state) & 3u) == 2);
    CHECK(retired == 31);
    CHECK(ap_collect(&q) == 0);
    CHECK(ap_pop(&q, lease, &v) == AP_RETRY);
    printf("OBS generation limit=%u admitted=%u terminal_retired=%u\n", limit,
           count, retired);
  }
}
static void registration(void) {
  struct ap_queue q;
  struct ap_lease leases[8], later;
  CHECK(ap_init(&q, 4, UINT_MAX >> 2, NULL, NULL) == AP_OK);
  for (unsigned i = 0; i < 4; i++)
    CHECK(ap_register(&q, &leases[i]) == AP_OK);
  later = leases[0];
  CHECK(ap_register(&q, &later) == AP_FULL && later.generation == 1);
  CHECK(ap_leave(&q, leases[0]) == AP_OK);
  CHECK(ap_register(&q, &later) == AP_OK && later.participant == 0 &&
        later.generation == 2);
  atomic_store(&q.hazard[0][0], atomic_load(&q.head));
  CHECK(ap_leave(&q, leases[0]) == AP_INVALID &&
        atomic_load(&q.hazard[0][0]) == 0);
  CHECK(ap_leave(&q, later) == AP_OK &&
        atomic_load(&q.hazard[0][0]) == AP_NONE);
  struct ap_lease final = {&q, 0, UINT_MAX >> 2};
  atomic_store(&q.registration[0], (final.generation << 2) | 1u);
  CHECK(ap_leave(&q, final) == AP_OK);
  CHECK((atomic_load(&q.registration[0]) & 3u) == 3u);
  CHECK(ap_register(&q, &later) == AP_FULL);
  struct ap_value v = value(99);
  CHECK(ap_pop(&q, leases[0], &v) == AP_INVALID && v.sequence == 99);
  puts("OBS registration generation-handshake stale-owner-no-clear saturation");
}
static void retention(void) {
  for (unsigned participants = 4; participants <= 8; participants += 4) {
    struct ap_queue q;
    struct ap_lease lease[8];
    CHECK(ap_init(&q, participants, UINT_MAX >> 2, NULL, NULL) == AP_OK);
    for (unsigned p = 0; p < participants; p++)
      CHECK(ap_register(&q, &lease[p]) == AP_OK);
    struct ap_lease actor = lease[participants - 1];
    struct ap_value v;
    unsigned held = 2 * (participants - 1);
    for (unsigned i = 0; i < held; i++) {
      CHECK(ap_push(&q, actor, value(i)) == AP_OK);
      unsigned old = atomic_load(&q.head);
      atomic_store(&q.hazard[i / 2][i % 2], old);
      CHECK(ap_pop(&q, actor, &v) == AP_OK && v.sequence == i);
    }
    CHECK(ap_collect(&q) == 0);
    unsigned admitted = 31 - held;
    for (unsigned i = 0; i < admitted; i++)
      CHECK(ap_push(&q, actor, value(100 + i)) == AP_OK);
    CHECK(ap_push(&q, actor, value(999)) == AP_FULL);
    atomic_store(&q.hazard[0][0], AP_NONE);
    CHECK(ap_collect(&q) == 1);
    CHECK(ap_push(&q, actor, value(999)) == AP_OK);
    printf("OBS retention participants=%u held_retired=%u "
           "queued_before_full=%u nodes=32\n",
           participants, held, admitted);
  }
}
static void *depart(void *ptr) {
  struct ag_operation *op = ptr;
  unsigned head = atomic_load(&op->q->head);
  atomic_store(&op->q->hazard[op->lease.participant][0], head);
  return NULL;
}
static void departure(void) {
  struct ap_queue q;
  struct ap_lease holder, actor, next;
  struct ap_value v;
  CHECK(ap_init(&q, 4, UINT_MAX >> 2, NULL, NULL) == AP_OK);
  CHECK(ap_register(&q, &holder) == AP_OK);
  CHECK(ap_register(&q, &actor) == AP_OK);
  CHECK(ap_push(&q, actor, value(11)) == AP_OK);
  struct ag_operation op = {&q, holder, 0, 0, {0, 0}};
  pthread_t thread;
  CHECK(!pthread_create(&thread, NULL, depart, &op));
  CHECK(!pthread_join(thread, NULL));
  CHECK(ap_pop(&q, actor, &v) == AP_OK);
  CHECK(ap_collect(&q) == 0);
  CHECK(ap_leave(&q, holder) == AP_OK);
  CHECK(ap_collect(&q) == 1);
  CHECK(ap_register(&q, &next) == AP_OK &&
        next.generation == holder.generation + 1);
  CHECK(ap_leave(&q, holder) == AP_INVALID);
  CHECK(ap_leave(&q, next) == AP_OK);
  puts("OBS departure actual-join-before-successor-clears-hazard; "
       "no-clearing-merely-paused-owner");
}
struct ag_stress {
  struct ap_queue *q;
  struct ap_lease lease;
  unsigned index, producers, total;
  atomic_uint *seen, *received, *done;
};
static void *stress_thread(void *ptr) {
  struct ag_stress *s = ptr;
  role = 0;
  if (s->index < s->producers) {
    for (unsigned i = 0; i < 512; i++) {
      unsigned n = s->index * 512 + i + 1;
      int r;
      while ((r = ap_push(s->q, s->lease, value(n))) == AP_FULL)
        ap_collect(s->q);
      CHECK(r == AP_OK);
    }
    atomic_fetch_add(s->done, 1);
  } else {
    while (atomic_load(s->received) < s->total) {
      struct ap_value v;
      int r = ap_pop(s->q, s->lease, &v);
      if (r == AP_RETRY) {
        ap_collect(s->q);
        continue;
      }
      CHECK(r == AP_OK && v.sequence >= 1 && v.sequence <= s->total &&
            v.inverse == ~v.sequence);
      CHECK(atomic_fetch_add(&s->seen[v.sequence - 1], 1) == 0);
      atomic_fetch_add(s->received, 1);
      ap_collect(s->q);
    }
  }
  CHECK(ap_leave(s->q, s->lease) == AP_OK);
  return NULL;
}
static void stress(void) {
  for (unsigned participants = 4; participants <= 8; participants += 4) {
    struct ap_queue q;
    CHECK(ap_init(&q, participants, UINT_MAX >> 2, NULL, NULL) == AP_OK);
    unsigned producers = participants / 2, total = producers * 512;
    atomic_uint seen[2048], received, done;
    atomic_init(&received, 0);
    atomic_init(&done, 0);
    for (unsigned i = 0; i < total; i++)
      atomic_init(&seen[i], 0);
    struct ag_stress args[8];
    pthread_t threads[8];
    for (unsigned i = 0; i < participants; i++) {
      struct ap_lease lease;
      CHECK(ap_register(&q, &lease) == AP_OK);
      args[i] =
          (struct ag_stress){&q, lease, i, producers, total, seen, &received, &done};
    }
    for (unsigned i = 0; i < participants; i++)
      CHECK(!pthread_create(&threads[i], NULL, stress_thread, &args[i]));
    for (unsigned i = 0; i < participants; i++)
      CHECK(!pthread_join(threads[i], NULL));
    CHECK(atomic_load(&done) == producers && atomic_load(&received) == total);
    for (unsigned i = 0; i < total; i++)
      CHECK(atomic_load(&seen[i]) == 1);
    ap_collect(&q);
    unsigned live = 0;
    for (unsigned n = 0; n < 32; n++) {
      unsigned state = atomic_load(&q.node[n].state);
      CHECK((state & 3u) == 0 || (state & 3u) == 1);
      live += (state & 3u) == 1;
    }
    CHECK(live == 1 && atomic_load(&q.head) == atomic_load(&q.tail));
    printf(
        "OBS stress participants=%u exact_once=%u joined=1 remaining_dummy=1\n",
        participants, total);
  }
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "validation"))
    validation();
  else if (!strcmp(argv[1], "protection"))
    protection();
  else if (!strcmp(argv[1], "publication"))
    publication();
  else if (!strcmp(argv[1], "collector"))
    collector();
  else if (!strcmp(argv[1], "registration"))
    registration();
  else if (!strcmp(argv[1], "generation"))
    generation();
  else if (!strcmp(argv[1], "retention"))
    retention();
  else if (!strcmp(argv[1], "departure"))
    departure();
  else if (!strcmp(argv[1], "stress"))
    stress();
  else
    return 2;
  return 0;
}
