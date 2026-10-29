#include "lc.h"
#include "test.h"
#include <string.h>
int main(void) {
  for (size_t cap = 0; cap <= 8; cap++)
    for (size_t n = 0; n <= 9; n++) {
      struct lc_buffer *p = lc_create(cap);
      CHECK(p);
      unsigned char in[10] = {1, 0, 3, 4, 5, 6, 7, 8, 9, 10}, out[20];
      memset(out, 99, sizeof out);
      size_t written = 87;
      CHECK(lc_append(p, in, n) == (n <= cap ? LC_OK : LC_FULL));
      size_t got = 999;
      const unsigned char *v = lc_view(p, &got);
      CHECK(v && got == (n <= cap ? n : 0));
      CHECK(!memcmp(v, in, got));
      if (got) {
        CHECK(lc_append(p, in, 9) == LC_FULL);
        CHECK(lc_view(p, &written) == v && written == got);
        CHECK(!memcmp(v, in, got));
        CHECK(lc_copy(p, out, got - 1, &written) == LC_FULL && out[0] == 99 &&
              written == got);
      }
      CHECK(lc_copy(p, out, sizeof out, &written) == LC_OK && written == got &&
            !memcmp(out, in, got));
      CHECK(lc_append(p, NULL, 0) == LC_OK);
      CHECK(lc_append(p, NULL, 1) == LC_INVALID || got == cap);
      lc_release_buffer(p);
    }
  struct lc_bytes needed = {77};
  CHECK(lc_extent((struct lc_elements){3}, 4, (struct lc_bytes){12}, &needed) == LC_OK &&
        needed.value == 12);
  CHECK(lc_extent((struct lc_elements){SIZE_MAX}, 2, (struct lc_bytes){SIZE_MAX}, &needed) ==
            LC_FULL &&
        needed.value == 12);
  CHECK(lc_extent((struct lc_elements){4}, 4, (struct lc_bytes){12}, &needed) == LC_FULL &&
        needed.value == 12);
  CHECK(lc_extent((struct lc_elements){0}, 0, (struct lc_bytes){12}, &needed) == LC_INVALID);
  puts("LC_BUFFER snapshots, embedded-zero, unit wrappers passed");
  return 0;
}
