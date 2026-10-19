#include "publication.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "PUBLICATION-CHECK:%d:%s\n", __LINE__, #x);              \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  AePublication *p;
  atomic_uint ready, done;
  unsigned bad, reads;
} Reader;
static void *reader(void *v) {
  Reader *r = v;
  atomic_store_explicit(&r->ready, 1, memory_order_release);
  do {
    AeReadLease l = {0};
    AeConfig c;
    if (ae_publication_acquire(r->p, &l) != AE_OK) {
      ++r->bad;
      break;
    }
    if (ae_publication_read(&l, &c) != AE_OK || c.count != 3 ||
        c.values[1] != c.values[0] + 1 || c.values[2] != c.values[0] + 2)
      ++r->bad;
    if (ae_publication_release(&l) != AE_OK)
      ++r->bad;
    ++r->reads;
  } while (!atomic_load_explicit(&r->done, memory_order_acquire));
  return 0;
}
int main(void) {
  AeConfig c = {{1, 2, 3}, 3, 1};
  AePublication *p = ae_publication_new(&c);
  CHECK(p);
  AeReadLease first = {0}, second = {0};
  CHECK(ae_publication_acquire(p, &first) == AE_OK);
  c = (AeConfig){{4, 5, 6}, 3, 2};
  CHECK(ae_publication_replace(p, &c, 0) == AE_OK);
  CHECK(ae_publication_acquire(p, &second) == AE_OK);
  c = (AeConfig){{7, 8, 9}, 3, 3};
  CHECK(ae_publication_replace(p, &c, 0) == AE_OK);
  unsigned made, freed, live;
  CHECK(ae_publication_counts(p, &made, &freed, &live) == AE_OK && made == 3 &&
        freed == 0 && live == 3);
  CHECK(ae_publication_replace(p, &c, 0) == AE_FULL);
  AeConfig old;
  CHECK(ae_publication_read(&first, &old) == AE_OK && old.values[0] == 1);
  CHECK(ae_publication_release(&first) == AE_OK &&
        ae_publication_release(&second) == AE_OK);
  CHECK(ae_publication_replace(p, &c, 1) == AE_EXHAUSTED);
  Reader r = {.p = p};
  pthread_t worker;
  CHECK(!pthread_create(&worker, 0, reader, &r));
  while (!atomic_load_explicit(&r.ready, memory_order_acquire)) {
  }
  for (unsigned i = 0; i < 1000; ++i) {
    unsigned x = i % 7;
    c = (AeConfig){{x, x + 1, x + 2}, 3, i + 4};
    CHECK(ae_publication_replace(p, &c, 0) == AE_OK);
  }
  atomic_store_explicit(&r.done, 1, memory_order_release);
  CHECK(!pthread_join(worker, 0) && !r.bad && r.reads);
  CHECK(ae_publication_acquire(p, &first) == AE_OK &&
        ae_publication_close(p) == AE_OK);
  CHECK(ae_publication_destroy(&p) == AE_AGAIN && p);
  CHECK(ae_publication_read(&first, &old) == AE_OK && old.version == 1003);
  CHECK(ae_publication_release(&first) == AE_OK);
  CHECK(ae_publication_counts(p, &made, &freed, &live) == AE_OK &&
        made == freed && !live);
  CHECK(ae_publication_destroy(&p) == AE_OK && !p);
  printf("PUBLICATION-PASS:replacements=1000;created=%u;destroyed=%u;reader_"
         "snapshots=%u;\n",
         made, freed, r.reads);
  return 0;
}
