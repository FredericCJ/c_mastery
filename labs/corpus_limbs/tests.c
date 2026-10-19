#include "limbs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "BL_CHECK:%d:%s\n", __LINE__, #x);                       \
      return 41;                                                               \
    }                                                                          \
  } while (0)
static int arithmetic(void) {
  uint16_t a[65], b[65], s[132], v[132], r[132];
  BlOutput o = {v, 132, 0}, rem = {r, 132, 0};
  for (size_t n = 1; n <= 64; n++) {
    for (size_t i = 0; i < n; i++)
      a[i] = (uint16_t)BL_MASK;
    b[0] = 1;
    CHECK(bl_add((BlView){a, n}, (BlView){b, 1}, &o, s, 132) == BL_OK);
    CHECK(o.used == n + 1 && v[n] == 1);
    for (size_t i = 0; i < n; i++)
      CHECK(v[i] == 0);
    for (size_t i = 0; i <= n; i++)
      a[i] = 0;
    a[n] = 1;
    CHECK(bl_sub((BlView){a, n + 1}, (BlView){b, 1}, &o, s, 132) == BL_OK);
    CHECK(o.used == n);
    for (size_t i = 0; i < n; i++)
      CHECK(v[i] == BL_MASK);
  }
  a[0] = (uint16_t)BL_MASK;
  b[0] = (uint16_t)BL_MASK;
  CHECK(bl_mul((BlView){a, 1}, (BlView){b, 1}, &o, s, 132) == BL_OK);
  CHECK(o.used == 2 && v[0] == 1 && v[1] == BL_MASK - 1);
  a[0] = 7;
  b[0] = 3;
  CHECK(bl_div((BlView){a, 1}, (BlView){b, 1}, &o, &rem, s, 132) == BL_OK &&
        o.used == 1 && v[0] == 2 && rem.used == 1 && r[0] == 1);
  a[0] = 6;
  CHECK(bl_div((BlView){a, 1}, (BlView){b, 1}, &o, &rem, s, 132) == BL_OK &&
        v[0] == 2 && rem.used == 0);
  a[0] = 1;
  v[0] = 91;
  o.used = 1;
  o.capacity = 0;
  CHECK(bl_add((BlView){a, 1}, (BlView){b, 1}, &o, s, 132) == BL_INVALID &&
        v[0] == 91 && o.used == 1);
  o.used = 0;
  CHECK(bl_add((BlView){a, 1}, (BlView){b, 1}, &o, s, 132) == BL_FULL &&
        v[0] == 91 && o.used == 0);
  o.capacity = 132;
  o.used = 1;
  rem.used = 1;
  r[0] = 92;
  CHECK(bl_div((BlView){a, 1}, (BlView){NULL, 0}, &o, &rem, s, 132) ==
            BL_ZERO_DIVISOR &&
        v[0] == 91 && r[0] == 92);
  CHECK(bl_div((BlView){a, 1}, (BlView){b, 1}, &o, &rem, s, 2) == BL_FULL &&
        v[0] == 91 && r[0] == 92);
  rem.capacity = 0;
  rem.used = 0;
  CHECK(bl_div((BlView){a, 1}, (BlView){b, 1}, &o, &rem, s, 132) == BL_FULL &&
        v[0] == 91 && o.used == 1);
  CHECK(bl_sub((BlView){a, 1}, (BlView){b, 1}, &o, s, 132) == BL_NEGATIVE &&
        v[0] == 91);
  a[1] = 0;
  CHECK(bl_add((BlView){a, 2}, (BlView){b, 1}, &o, s, 132) == BL_INVALID &&
        v[0] == 91);
  BlOutput empty = {NULL, 0, 0};
  CHECK(bl_mul((BlView){NULL, 0}, (BlView){NULL, 0}, &empty, NULL, 0) ==
            BL_OK &&
        empty.used == 0);
  CHECK(bl_add((BlView){NULL, 0}, (BlView){NULL, 0}, &empty, s, 1) == BL_OK &&
        empty.used == 0);
  return 0;
}
typedef struct {
  void *p[4];
  size_t n[4], live, attempt;
  int fail;
} Alloc;
static uint16_t *allocate(void *arg, size_t n) {
  Alloc *a = arg;
  a->attempt++;
  if (a->fail)
    return NULL;
  for (size_t i = 0; i < 4; i++)
    if (!a->p[i]) {
      a->p[i] = malloc(n * sizeof(uint16_t));
      a->n[i] = n;
      if (a->p[i])
        a->live++;
      return a->p[i];
    }
  return NULL;
}
static void release(void *arg, uint16_t *p, size_t n) {
  Alloc *a = arg;
  for (size_t i = 0; i < 4; i++)
    if (a->p[i] == p && a->n[i] == n) {
      free(p);
      a->p[i] = NULL;
      a->live--;
      return;
    }
  abort();
}
static int ownership(void) {
  uint16_t a[2] = {7, 1}, b[2] = {3, 1}, s[8], qv[4], rv[4];
  BlSignedOutput q = {0, {qv, 4, 0}}, r = {0, {rv, 4, 0}};
  CHECK(bl_signed_div((BlSigned){-1, {a, 2}}, (BlSigned){1, {b, 2}}, &q, &r, s,
                      8) == BL_OK);
  CHECK(q.sign == -1 && q.magnitude.used == 1 && qv[0] == 1 && r.sign == -1 &&
        rv[0] == 4);
  CHECK(bl_signed_add((BlSigned){-1, {a, 2}}, (BlSigned){1, {a, 2}}, &q, s,
                      8) == BL_OK &&
        q.sign == 0 && q.magnitude.used == 0);
  CHECK(bl_signed_add((BlSigned){0, {a, 2}}, (BlSigned){1, {b, 2}}, &q, s, 8) ==
        BL_INVALID);
  for (int sa = -1; sa <= 1; sa += 2) {
    for (int sb = -1; sb <= 1; sb += 2) {
      CHECK(bl_signed_div((BlSigned){sa, {a, 1}}, (BlSigned){sb, {b, 1}}, &q,
                          &r, s, 8) == BL_OK);
      CHECK(q.sign == sa * sb && q.magnitude.used == 1 && qv[0] == 2 &&
            r.sign == sa && r.magnitude.used == 1 && rv[0] == 1);
    }
  }
  Alloc allocator = {{NULL}, {0}, 0, 0, 0};
  BlOwner o;
  CHECK(bl_owner_init(&o, &allocator, allocate, release) == BL_OK);
  CHECK(bl_owner_set(&o, (BlView){a, 2}, SIZE_MAX) == BL_FULL &&
        allocator.attempt == 0 && o.digits == NULL);
  CHECK(bl_owner_set(&o, (BlView){a, 2}, 4) == BL_OK && allocator.live == 1);
  uint16_t *old = o.digits;
  allocator.fail = 1;
  CHECK(bl_owner_set(&o, (BlView){b, 2}, 8) == BL_ALLOCATION &&
        o.digits == old && old[0] == 7 && o.capacity == 4);
  allocator.fail = 0;
  CHECK(bl_owner_set(&o, (BlView){o.digits, o.used}, 8) == BL_OK &&
        o.capacity == 8 && o.digits[0] == 7);
  /* Do not compare or dereference the revoked old pointer after replacement. */
  old = NULL;
  int correct = allocator.live == 1;
  bl_owner_drop(&o);
  bl_owner_drop(&o);
  correct = correct && allocator.live == 0 && o.used == 0 && o.capacity == 0 &&
            !o.digits;
  /* Test teardown releases a deliberately leaked mutant allocation, so an
     intended semantic rejection cannot be confused with a sanitizer finding. */
  for (size_t i = 0; i < 4; i++)
    if (allocator.p[i])
      free(allocator.p[i]);
  CHECK(correct);
  return 0;
}
static size_t parse(const char *text, uint16_t *a) {
  size_t n = 0;
  const char *p = text;
  char *end;
  while (*p) {
    unsigned long x = strtoul(p, &end, 16);
    if (end == p || x > BL_MASK || n == 256)
      exit(42);
    a[n++] = (uint16_t)x;
    p = end;
    if (*p == ',')
      p++;
    else if (*p)
      exit(42);
  }
  return n;
}
static void print(const BlOutput *o) {
  printf("%zu", o->used);
  for (size_t i = 0; i < o->used; i++)
    printf(" %u", (unsigned)o->digits[i]);
}
static int driver(void) {
  char line[6000], left[2500], right[2500], op;
  uint16_t a[256], b[256], s[514], q[512], r[512];
  while (fgets(line, sizeof line, stdin)) {
    if (sscanf(line, "%c %2499s %2499s", &op, left, right) != 3)
      return 42;
    BlView x = {a, strcmp(left, "-") ? parse(left, a) : 0},
           y = {b, strcmp(right, "-") ? parse(right, b) : 0};
    BlOutput qo = {q, 512, 0}, ro = {r, 512, 0};
    BlStatus status;
    if (op == '+')
      status = bl_add(x, y, &qo, s, 514);
    else if (op == '-')
      status = bl_sub(x, y, &qo, s, 514);
    else if (op == '*')
      status = bl_mul(x, y, &qo, s, 514);
    else if (op == '/')
      status = bl_div(x, y, &qo, &ro, s, 514);
    else
      return 42;
    printf("%u ", (unsigned)status);
    print(&qo);
    printf(" | ");
    print(&ro);
    putchar('\n');
  }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 42;
  if (!strcmp(argv[1], "arithmetic"))
    return arithmetic();
  if (!strcmp(argv[1], "ownership"))
    return ownership();
  if (!strcmp(argv[1], "driver"))
    return driver();
  return 42;
}
