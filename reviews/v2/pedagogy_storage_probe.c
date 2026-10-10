/* Independent finite prefix/embedded-zero ordering review; RAM backend has
 * immediate persistence and does not test crash durability. */
#include "store.h"
#include <stdio.h>
#include <string.h>
static unsigned char image[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
static int rd(void *ctx, uint32_t page, void *out) {
  (void)ctx;
  if (page >= STORE_IMAGE_PAGES) return 1;
  memcpy(out, image[page], STORE_PAGE_SIZE); return 0;
}
static int wr(void *ctx, uint32_t page, const void *in) {
  (void)ctx;
  if (page >= STORE_IMAGE_PAGES) return 1;
  memcpy(image[page], in, STORE_PAGE_SIZE); return 0;
}
static int sync_image(void *ctx) { (void)ctx; return 0; }
static size_t make_key(unsigned i, unsigned char key[2]) {
  key[0] = (unsigned char)(i / 2); key[1] = 0; return 1 + (i % 2);
}
static unsigned char present[64];
static unsigned next;
static int bad;
static int visit(void *ctx, const uint8_t *key, size_t size,
                 const uint8_t *value, size_t value_size) {
  unsigned char wanted[2];
  (void)ctx;
  while (next < 64 && !present[next]) next++;
  if (next >= 64 || size != make_key(next, wanted) || memcmp(wanted, key, size) ||
      value_size != 1 || value[0] != next) { bad = 1; return 1; }
  next++; return 0;
}
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); return 1; } } while (0)
static int validate(store *s) {
  next = 0; bad = 0;
  CHECK(store_check(s) == STORE_OK);
  CHECK(store_scan(s, NULL, 0, visit, NULL) == STORE_OK && !bad);
  while (next < 64 && !present[next]) next++;
  CHECK(next == 64);
  for (unsigned i = 0; i < 64; i++) {
    unsigned char key[2], value = 200;
    size_t length = 500;
    int result = store_get(s, key, make_key(i, key), &value, 1, &length);
    CHECK(result == (present[i] ? STORE_OK : STORE_NOT_FOUND));
    CHECK(present[i] ? (value == i && length == 1) : (value == 200 && length == 500));
  }
  return 0;
}
int main(void) {
  static store s;
  store_io io = {NULL, rd, wr, sync_image, STORE_IMAGE_PAGES};
  CHECK(store_format(io) == STORE_OK);
  CHECK(store_open(&s, io) == STORE_OK);
  CHECK(store_begin(&s) == STORE_OK);
  for (unsigned j = 64; j > 0; j--) {
    unsigned i = j - 1;
    unsigned char key[2], value = (unsigned char)i;
    CHECK(store_put(&s, key, make_key(i, key), &value, 1) == STORE_OK);
    present[i] = 1;
  }
  CHECK(validate(&s) == 0);
  CHECK(store_commit(&s) == STORE_OK);
  CHECK(store_open(&s, io) == STORE_OK);
  CHECK(validate(&s) == 0);
  CHECK(store_begin(&s) == STORE_OK);
  for (unsigned i = 0; i < 64; i += 3) {
    unsigned char key[2];
    CHECK(store_delete(&s, key, make_key(i, key)) == STORE_OK);
    present[i] = 0;
    CHECK(validate(&s) == 0);
  }
  CHECK(store_commit(&s) == STORE_OK);
  CHECK(store_open(&s, io) == STORE_OK);
  CHECK(validate(&s) == 0);
  puts("64 prefix/embedded-zero keys, reverse insertion, interleaved deletion, complete maps and reopen: PASS");
  return 0;
}
