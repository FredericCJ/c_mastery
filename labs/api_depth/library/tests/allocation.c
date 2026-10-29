#include "lc.h"
#include "test.h"
static unsigned calls, live, fail;
void *__real_malloc(size_t);
void __real_free(void *);
void *__wrap_malloc(size_t n) {
  calls++;
  if (calls == fail)
    return NULL;
  void *p = __real_malloc(n);
  if (p)
    live++;
  return p;
}
void __wrap_free(void *p) {
  if (p)
    live--;
  __real_free(p);
}
static void done(void *p, enum lc_status s) {
  (void)p;
  (void)s;
}
int main(void) {
  fail = 1;
  CHECK(lc_create(4) == NULL && live == 0);
  fail = 2;
  CHECK(lc_start(done, NULL) == NULL && live == 0);
  fail = 0;
  struct lc_buffer *b = lc_create(0);
  CHECK(b && live == 1);
  lc_release_buffer(b);
  CHECK(!live);
  struct lc_operation *p = lc_start(done, NULL);
  CHECK(p && live == 1);
  CHECK(lc_poll(p) == LC_OK && lc_release_operation(p) == LC_OK && !live);
  puts("LC_ALLOCATION actual malloc failure and paired free passed");
  return 0;
}
