#include "test.h"
#include <stddef.h>
int main(void) {
  NDStore s = {0};
  char a[24] = {"alpha"}, b[24] = {"beta"}, c[24] = {"gamma"};
  unsigned x, y, z;
  CHECK(nd_insert(&s, 1, 2, a, &x) == 0);
  CHECK(nd_insert(&s, 3, 4, b, &y) == 0);
  CHECK(nd_insert(&s, 5, 6, c, &z) == 0);
  CHECK(nd_remove(&s, y) == 0);
  NDRecord r;
  CHECK(nd_record(&s, z, &r) == 0 && r.x == 5 && !strcmp(r.label, "gamma"));
  unsigned order[] = {1, 0};
  CHECK(nd_reorder(&s, order, 2) == 0);
  CHECK(nd_record(&s, x, &r) == 0 && r.x == 1 && !strcmp(r.label, "alpha"));
  NDStore old = s;
  unsigned bad[] = {0, 0};
  CHECK(nd_reorder(&s, bad, 2) == ND_BAD && memcmp(&s, &old, sizeof s) == 0);
  CHECK(nd_record(&s, y, &r) == ND_BAD);
  for (unsigned i = 0; i < 14; i++)
    CHECK(nd_insert(&s, i, -(double)i, a, &y) == 0);
  old = s;
  CHECK(nd_insert(&s, 1, 2, b, &y) == ND_BAD &&
        memcmp(&s, &old, sizeof s) == 0);
  double ao = 0, so = 0;
  for (size_t i = 0; i < s.count; i++) {
    CHECK(nd_record(&s, s.id[i], &r) == 0);
    ao += r.x;
    so += s.x[i];
  }
  CHECK(ao == so);
  printf("PASS hot/cold identity, reorder and bounded capacity; "
         "sizeof(record)=%zu align=%zu offsets=%zu,%zu,%zu,%zu\n",
         sizeof(NDRecord), _Alignof(NDRecord), offsetof(NDRecord, x),
         offsetof(NDRecord, y), offsetof(NDRecord, id),
         offsetof(NDRecord, label));
}
