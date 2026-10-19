#include "test.h"
#include <pthread.h>
typedef struct {
  ArMachine *m;
  pthread_mutex_t *mutex;
  ArEvent event;
  int result;
} Producer;
static void *produce(void *p) {
  Producer *x = p;
  CHECK(pthread_mutex_lock(x->mutex) == 0);
  x->result = ar_event(x->m, x->event);
  CHECK(pthread_mutex_unlock(x->mutex) == 0);
  return 0;
}
int main(void) {
  ArMachine m;
  ar_machine_init(&m);
  uint32_t id;
  CHECK(ar_acquire(&m, &id) == AR_OK && id == 1);
  CHECK(ar_machine_cancel(&m) == AR_CANCELLED && m.releases == 1);
  CHECK(ar_acquire(&m, &id) == AR_OK && id == 2);
  CHECK(ar_submit(&m) == AR_OK);
  ArEvent e = {m.incarnation, id, 0};
  CHECK(ar_event(&m, e) == AR_OK && ar_event(&m, e) == AR_OK &&
        ar_event(&m, e) == AR_FULL);
  CHECK(ar_machine_cancel(&m) == AR_PENDING);
  ArEvent ack = e;
  ack.type = 1;
  CHECK(ar_event(&m, ack) == AR_OK);
  CHECK(ar_step(&m) == AR_CANCELLED && m.releases == 2);
  CHECK(ar_step(&m) == AR_STATE && ar_step(&m) == AR_STATE && m.releases == 2);
  CHECK(ar_restart(&m) == AR_OK && m.incarnation == 2);
  CHECK(ar_acquire(&m, &id) == AR_OK && ar_submit(&m) == AR_OK);
  CHECK(ar_event(&m, e) == AR_STATE);
  e = (ArEvent){m.incarnation, id, 0};
  CHECK(ar_event(&m, e) == AR_OK && ar_step(&m) == AR_OK);
  CHECK(ar_machine_cancel(&m) == AR_STATE && m.releases == 3);
  for (unsigned order = 0; order < 2; ++order) {
    ArMachine independent[2];
    uint32_t ids[2];
    for (unsigned i = 0; i < 2; ++i) {
      ar_machine_init(&independent[i]);
      CHECK(ar_acquire(&independent[i], &ids[i]) == AR_OK &&
            ar_submit(&independent[i]) == AR_OK);
    }
    for (unsigned j = 0; j < 2; ++j) {
      unsigned i = j ^ order;
      CHECK(ar_event(&independent[i], (ArEvent){1, ids[i], 0}) == AR_OK &&
            ar_step(&independent[i]) == AR_OK);
    }
    CHECK(independent[0].releases == 1 && independent[1].releases == 1);
  }
  CHECK(ar_acquire(&m, &id) == AR_OK && ar_submit(&m) == AR_OK);
  pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  Producer p[2] = {{&m, &lock, {m.incarnation, id, 0}, -1},
                   {&m, &lock, {m.incarnation, id, 0}, -1}};
  pthread_t workers[2];
  for (unsigned i = 0; i < 2; ++i)
    CHECK(!pthread_create(&workers[i], 0, produce, &p[i]));
  for (unsigned i = 0; i < 2; ++i)
    CHECK(!pthread_join(workers[i], 0) && p[i].result == AR_OK);
  CHECK(ar_step(&m) == AR_OK && ar_step(&m) == AR_STATE && m.releases == 4);
  CHECK(!pthread_mutex_destroy(&lock));
  m.incarnation = UINT32_MAX;
  CHECK(ar_restart(&m) == AR_STATE);
  puts("PASS machine retained owner,incarnation,priority acknowledgement,stale "
       "events,joined producers");
}
