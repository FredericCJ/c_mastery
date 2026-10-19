#include "allocation.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "ALLOCATION-CHECK:%d:%s\n", __LINE__, #x);               \
      return 1;                                                                \
    }                                                                          \
  } while (0)
/* Original bounded backend: no pointer is freed by its zero-size realloc path.
   Live slots make ownership assertions safe even for semantic mutants. */
struct slot {
  _Alignas(64) unsigned char data[256];
  int live;
};
struct backend {
  struct slot slots[3];
  unsigned allocations, resizes, releases, copies, zero_calls, fail_alloc,
      fail_resize, copy_after, bad_release;
};
static int supports(void *v, size_t a) {
  (void)v;
  return a == 1 || a == _Alignof(max_align_t) || a == 64;
}
static void *allocate(void *v, size_t n, size_t a) {
  struct backend *b = v;
  ++b->allocations;
  if (!n) {
    ++b->zero_calls;
    return NULL;
  }
  if (b->fail_alloc || n > 256 || !supports(v, a))
    return NULL;
  for (unsigned i = 0; i < 3; ++i)
    if (!b->slots[i].live) {
      b->slots[i].live = 1;
      memset(b->slots[i].data, 0xa5, 256);
      return b->slots[i].data;
    }
  return NULL;
}
static void release(void *v, void *p) {
  struct backend *b = v;
  for (unsigned i = 0; i < 3; ++i)
    if (b->slots[i].data == p) {
      if (!b->slots[i].live)
        b->bad_release = 1;
      else {
        b->slots[i].live = 0;
        ++b->releases;
      }
      return;
    }
  b->bad_release = 1;
}
static void *resize(void *v, void *p, size_t n) {
  struct backend *b = v;
  ++b->resizes;
  if (!n) {
    ++b->zero_calls;
    return NULL;
  }
  if (b->fail_resize || n > 256)
    return NULL; /* Same storage, all old bytes stay valid. */
  return p;
}
static int copy(void *v, void *d, const void *s, size_t n) {
  struct backend *b = v;
  ++b->copies;
  for (size_t i = 0; i < n; ++i) {
    if (b->copy_after == i)
      return 1;
    ((unsigned char *)d)[i] = ((const unsigned char *)s)[i];
  }
  return b->copy_after == n;
}
static unsigned active(struct backend *b) {
  return (unsigned)b->slots[0].live + (unsigned)b->slots[1].live +
         (unsigned)b->slots[2].live;
}
static struct la_ops operations(struct backend *b) {
  return (struct la_ops){
      b, supports, allocate, resize, release, copy, _Alignof(max_align_t)};
}
int main(void) {
  struct backend b = {.copy_after = 999};
  struct la_ops ops = operations(&b);
  struct la_owner o = {0};
  CHECK(la_create(ops, &o, SIZE_MAX, 2, 1) == LA_RANGE && b.allocations == 0 &&
        o.data == NULL);
  CHECK(la_create(ops, &o, SIZE_MAX, 1, 64) == LA_RANGE && b.allocations == 0);
  CHECK(la_create(ops, &o, 0, 1, 1) == LA_ARGUMENT && b.allocations == 0);
  CHECK(la_create(ops, &o, 1, 1, 3) == LA_ALIGNMENT && b.allocations == 0);
  CHECK(la_create(ops, &o, 1, 1, 128) == LA_ALIGNMENT && b.allocations == 0);
  b.fail_alloc = 1;
  CHECK(la_create(ops, &o, 4, 1, 1) == LA_NOMEM && o.data == NULL &&
        active(&b) == 0);
  b.fail_alloc = 0;
  CHECK(la_create(ops, &o, 4, 1, 1) == LA_OK && o.length == 4 &&
        o.capacity == 4);
  memcpy(o.data, "ABCD", 4);
  void *saved = o.data;
  CHECK(la_resize_native(ops, &o, 0, 1) == LA_ARGUMENT && b.resizes == 0 &&
        b.zero_calls == 0 && o.data == saved && active(&b) == 1);
  b.fail_resize = 1;
  CHECK(la_resize_native(ops, &o, 8, 1) == LA_NOMEM && o.data == saved &&
        o.length == 4 && !memcmp(o.data, "ABCD", 4) && active(&b) == 1);
  b.fail_resize = 0;
  CHECK(la_resize_native(ops, &o, 8, 1) == LA_OK && o.length == 8 &&
        !memcmp(o.data, "ABCD", 4));
  CHECK(la_release(ops, &o) == LA_OK && active(&b) == 0 && b.releases == 1 &&
        o.data == NULL);
  CHECK(la_release(ops, &o) == LA_OK && b.releases == 1 && !b.bad_release);
  for (unsigned fail = 0; fail <= 4; ++fail) {
    b = (struct backend){.copy_after = fail};
    ops = operations(&b);
    CHECK(la_create(ops, &o, 4, 1, 1) == LA_OK);
    memcpy(o.data, "ABCD", 4);
    saved = o.data;
    CHECK(la_resize_copy(ops, &o, 8, 1) == LA_COPY_FAILED && o.data == saved &&
          o.length == 4 && !memcmp(o.data, "ABCD", 4) && active(&b) == 1 &&
          b.releases == 1);
    b.copy_after = 999;
    CHECK(la_resize_copy(ops, &o, 8, 1) == LA_OK && o.length == 8 &&
          !memcmp(o.data, "ABCD", 4) && active(&b) == 1 && b.releases == 2);
    CHECK(la_release(ops, &o) == LA_OK && active(&b) == 0 && b.releases == 3 &&
          !b.bad_release);
  }
  b = (struct backend){.copy_after = 999};
  ops = operations(&b);
  CHECK(la_create(ops, &o, 4, 1, 64) == LA_OK && o.capacity == 64 &&
        o.alignment == 64 && (uintptr_t)o.data % 64 == 0);
  memcpy(o.data, "ABCD", 4);
  CHECK(la_resize_native(ops, &o, 8, 1) == LA_ALIGNMENT && b.resizes == 0);
  b.fail_alloc = 1;
  saved = o.data;
  CHECK(la_resize_copy(ops, &o, 65, 1) == LA_NOMEM && o.data == saved &&
        !memcmp(o.data, "ABCD", 4) && active(&b) == 1);
  b.fail_alloc = 0;
  CHECK(la_resize_copy(ops, &o, 65, 1) == LA_OK && o.capacity == 128 &&
        o.length == 65 && (uintptr_t)o.data % 64 == 0 &&
        !memcmp(o.data, "ABCD", 4));
  CHECK(la_resize_copy(ops, &o, 2, 1) == LA_OK && o.length == 2 &&
        !memcmp(o.data, "AB", 2));
  CHECK(la_release(ops, &o) == LA_OK && active(&b) == 0 && !b.zero_calls &&
        !b.bad_release);
  /* Exercise the adverse external callback separately, without ambiguity: its
     contract expressly retains the slot when returning NULL for size zero. */
  b = (struct backend){.copy_after = 999};
  ops = operations(&b);
  saved = allocate(&b, 4, 1);
  CHECK(saved && resize(&b, saved, 0) == NULL && active(&b) == 1);
  release(&b, saved);
  CHECK(active(&b) == 0 && b.zero_calls == 1 && b.releases == 1);
  puts("ALLOCATION-PASS:overflow:zero-release:resize:fallible-copy:aligned-"
       "owner");
  return 0;
}
