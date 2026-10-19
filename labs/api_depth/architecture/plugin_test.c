#include "registry.h"
#include "test.h"
typedef struct {
  unsigned calls, drops;
} Plugin;
static void release(void *p) { ++((Plugin *)p)->drops; }
static void call(void *p, AsRegistry *r, ApiHandle handle) {
  Plugin *x = p;
  ++x->calls;
  CHECK(as_remove(r, handle) == API_PENDING);
}
int main(void) {
  AsRegistry r;
  as_registry_init(&r, 1, 0, 0);
  Plugin p = {0};
  ApiHandle h;
  CHECK(as_register(&r, call, &p, release, &h) == API_OK && p.calls == 0);
  CHECK(as_dispatch(&r) == API_OK && p.calls == 1 && p.drops == 1);
  CHECK(as_dispatch(&r) == API_OK && p.calls == 1);
  CHECK(as_destroy(&r) == API_OK);
  puts("PASS actual registry registration transfers context,execution callback "
       "removes,deferred release");
}
