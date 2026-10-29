#include "foundation.h"
#include "ring.h"
#include "test.h"
int main(void) {
  unsigned char bytes[9];
  int32_t values[] = {1, 3, 5};
  size_t extent = 99;
  CHECK(f_checked_size(3, 3, 0, &extent) == F_OK && extent == 9);
  for (unsigned i = 0; i < 3; ++i)
    CHECK(ar_encode(1, (struct ar_record){values[i]}, bytes + 3 * i, 3) == AR_OK);
  struct f_cursor cursor = {bytes, extent, 0};
  struct ar_ring_policy ring;
  CHECK(ar_ring_init(&ring, 3) == AR_OK);
  for (unsigned i = 0; i < 3; ++i) {
    const uint8_t *span = 0;
    CHECK(f_take(&cursor, 3, &span) == F_OK);
    struct ar_record record;
    CHECK(ar_decode(1, span, 3, &record) == AR_OK);
    struct ar_eviction eviction;
    CHECK(ar_ring_push(&ring, (struct ar_item){(unsigned)record.value, 0}, 0,
                       &eviction) == AR_OK &&
          !eviction.valid);
  }
  const uint8_t *sentinel = bytes;
  CHECK(f_take(&cursor, 1, &sentinel) == F_RANGE && cursor.position == 9 &&
        sentinel == bytes);
  for (unsigned i = 0; i < 3; ++i) {
    struct ar_item item;
    CHECK(ar_ring_pop(&ring, &item) == AR_OK);
    int32_t input = (int32_t)item.id, out = 0;
    CHECK(ar_double(&input, 1, &out) == AR_OK && out == 2 * values[i]);
  }
  extent = 77;
  CHECK(f_checked_size(SIZE_MAX, 3, 1, &extent) == F_RANGE && extent == 77);
  puts("PASS actual checked extent,cursor,CDRing,format and checked transform "
       "composition");
}
