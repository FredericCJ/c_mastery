#include "v4_algo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "line %d: %s\n", __LINE__, #x);                          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  int fail, releases;
} Alloc;
static int32_t *take(void *p, size_t n) {
  return ((Alloc *)p)->fail ? NULL : malloc(n * sizeof(int32_t));
}
static void release(void *p, int32_t *a) {
  ((Alloc *)p)->releases++;
  free(a);
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "array")) {
    Alloc p = {0};
    V4Array a = {0};
    a.allocator = (V4Allocator){&p, take, release};
    CHECK(v4_reserve(&a, 2) == 0);
    a.data[0] = 41;
    a.length = 1;
    int32_t *old = a.data;
    p.fail = 1;
    CHECK(v4_reserve(&a, 8) != 0 && a.data == old && a.capacity == 2 &&
          a.data[0] == 41 && p.releases == 0);
    p.fail = 0;
    CHECK(v4_reserve(&a, 8) == 0 && a.length == 1 && a.data[0] == 41 &&
          p.releases == 1);
    CHECK(v4_reserve(&a, SIZE_MAX) != 0);
    release(&p, a.data);
  } else if (!strcmp(argv[1], "list")) {
    V4List q = {.head = -1, .tail = -1};
    int32_t x;
    CHECK(v4_list_pop(&q, &x) != 0);
    for (int i = 0; i < 8; i++)
      CHECK(v4_list_push(&q, i) == 0);
    CHECK(v4_list_push(&q, 99) != 0);
    for (int i = 0; i < 8; i++) {
      CHECK(v4_list_pop(&q, &x) == 0 && x == i);
      CHECK(v4_list_push(&q, i + 8) == 0);
    }
    for (int i = 8; i < 16; i++)
      CHECK(v4_list_pop(&q, &x) == 0 && x == i);
    CHECK(q.used == 0 && q.head == -1 && q.tail == -1);
  } else if (!strcmp(argv[1], "freelist")) {
    V4Free f = {{0}, {64}, 1};
    unsigned a, b;
    CHECK(v4_free_take(&f, 16, &a) == 0 && a == 0);
    CHECK(v4_free_take(&f, 16, &b) == 0 && b == 16);
    CHECK(v4_free_return(&f, a, 16) == 0);
    CHECK(v4_free_return(&f, b, 16) == 0 && f.count == 1 && f.length[0] == 64);
    V4Free old = f;
    CHECK(v4_free_return(&f, 4, 8) != 0 && !memcmp(&old, &f, sizeof f));
    CHECK(v4_free_take(&f, 65, &a) != 0);
  } else if (!strcmp(argv[1], "radix")) {
    uint32_t a[] = {UINT32_MAX, 0, 256, 255, 256, 1, 0x80000000}, tmp[7];
    uint32_t expected[] = {0, 1, 255, 256, 256, 0x80000000, UINT32_MAX};
    CHECK(v4_radix(a, 7, tmp, 6) != 0);
    CHECK(v4_radix(a, 7, tmp, 7) == 0 && !memcmp(a, expected, sizeof a));
  } else if (!strcmp(argv[1], "dp")) {
    unsigned w[] = {2, 3, 4}, v[] = {3, 4, 5}, out = 99;
    CHECK(v4_knapsack(w, v, 3, 5, &out) == 0 && out == 7);
    CHECK(v4_knapsack(w, v, 1, 4, &out) == 0 && out == 3);
    w[0] = 0;
    out = 99;
    CHECK(v4_knapsack(w, v, 3, 5, &out) != 0 && out == 99);
  } else if (!strcmp(argv[1], "filter")) {
    V4Filter f = {0};
    int32_t fir, iir;
    int32_t a[] = {4, 0, 0, 0}, ef[] = {1, 2, 1, 0}, ei[] = {0, 1, 1, 0};
    for (unsigned j = 0; j < 4; j++)
      CHECK(v4_filter(&f, a[j], &fir, &iir) == 0 && fir == ef[j] &&
            iir == ei[j]);
    f = (V4Filter){0};
    CHECK(v4_filter(&f, INT32_MAX, &fir, &iir) == 0 && fir == INT32_MAX / 4);
  } else
    return 2;
  return 0;
}
