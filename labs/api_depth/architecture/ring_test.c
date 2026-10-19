#include "ring.h"
#include "test.h"
int main(void) {
  unsigned histories = 0;
  for (unsigned policy = 0; policy < 3; ++policy)
    for (unsigned encoding = 0; encoding < 4096; ++encoding) {
      ArQueue model;
      ArRingPolicy actual;
      CHECK(ar_queue_init(&model, 2) == AR_OK &&
            ar_ring_init(&actual, 2) == AR_OK);
      unsigned code = encoding;
      for (unsigned k = 0; k < 6; ++k) {
        unsigned op = code % 4;
        code /= 4;
        if (op == 3) {
          ArItem a = {99, 99}, b = a;
          int x = ar_pop(&model, &a), y = ar_ring_pop(&actual, &b);
          CHECK(x == y && a.id == b.id && a.priority == b.priority);
        } else {
          ArItem v = {k + 1, op};
          ArEviction a = {9, {99, 99}}, b = a;
          int x = ar_push(&model, v, policy, &a),
              y = ar_ring_push(&actual, v, policy, &b);
          CHECK(x == y && a.valid == b.valid && a.item.id == b.item.id &&
                a.item.priority == b.item.priority);
        }
        CHECK(model.length == actual.ring.count);
      }
      ++histories;
    }
  printf("PASS actual CDRing policy adapter histories=%u payload association "
         "and eviction ownership\n",
         histories);
}
