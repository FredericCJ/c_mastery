#include "borrow.h"
#include "test.h"
int main(void) {
  unsigned char p[] = {1, 2, 3}, out[3] = {0};
  ArBorrow b = {0};
  size_t n = 99;
  CHECK(ar_borrow(&b, p, 3) == AR_OK && b.data == p);
  CHECK(ar_borrow_chunk(&b, 1, out, &n) == AR_PENDING && n == 1 &&
        out[0] == 1 && b.active);
  CHECK(ar_borrow_cancel(&b) == AR_PENDING && b.active);
  CHECK(ar_borrow_chunk(&b, 2, out + 1, &n) == AR_STATE);
  CHECK(ar_borrow_ack(&b) == AR_CANCELLED && !b.active && !b.data &&
        b.offset == 1);
  p[0] = 9;
  CHECK(ar_borrow(&b, p, 3) == AR_OK);
  CHECK(ar_borrow_chunk(&b, 2, out, &n) == AR_PENDING && n == 2);
  CHECK(ar_borrow_chunk(&b, 1, out + 2, &n) == AR_OK && n == 1 && !b.active &&
        !memcmp(p, out, 3));
  puts("PASS borrowed view retained to terminal or acknowledged cancel,retry "
       "begins a new record");
}
