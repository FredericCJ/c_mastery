#include "board.h"
#include "test.h"
struct ac_device {
  uint32_t words[16], last;
  uintptr_t address;
  unsigned reads, writes;
};
static uint32_t read_reg(void *p, uintptr_t a) {
  struct ac_device *d = p;
  ++d->reads;
  return d->words[a / 4];
}
static void write_reg(void *p, uintptr_t a, uint32_t v) {
  struct ac_device *d = p;
  ++d->writes;
  d->last = v;
  d->address = a;
  if (a == 60)
    d->words[a / 4] &= ~v;
  else
    d->words[a / 4] = v;
}
int main(void) {
  struct ac_device d = {0};
  struct ar_board high = {&d, read_reg, write_reg, 0, 2, 0},
          low = {&d, read_reg, write_reg, 0, 3, 1};
  CHECK(ar_board_init(&high) == AR_OK && ar_board_init(&low) == AR_OK);
  CHECK((d.words[0] & 0xf0) == 0x50);
  int32_t milli;
  CHECK(ar_centimilli(12, &milli) == AR_OK && milli == 120);
  CHECK(ar_service(milli, 100, (struct ar_indicator){&high, ar_board_set}) == AR_OK &&
        d.last == 4);
  CHECK(ar_service(milli, 100, (struct ar_indicator){&low, ar_board_set}) == AR_OK &&
        d.last == (8u << 16));
  CHECK(ar_service(99, 100, (struct ar_indicator){&low, ar_board_set}) == AR_OK &&
        d.last == 8);
  d.words[15] = 3;
  unsigned reads = d.reads;
  CHECK(ar_clear_pending(&high, 60, 1) == AR_OK && d.words[15] == 2 &&
        d.reads == reads);
  milli = 77;
  CHECK(ar_centimilli(INT32_MAX, &milli) == AR_RANGE && milli == 77);
  puts("PASS indicator logical units,two polarities,exclusive init,W1C direct "
       "mask");
}
