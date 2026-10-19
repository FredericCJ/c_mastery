#include "test.h"
int main(void) {
  ArField f[] = {{1, 1}, {2, 0}, {3, 0}, {4, 20}};
  ArConfig c = {.limit = 77};
  CHECK(ar_config(f, 4, &c) == AR_OK && c.limit == 0);
  for (unsigned n = 0; n <= AR_CAP; ++n) {
    f[1].value = n;
    CHECK(ar_config(f, 4, &c) == AR_OK && c.limit == n);
  }
  f[1].value = AR_CAP + 1;
  ArConfig old = c;
  CHECK(ar_config(f, 4, &c) == AR_RANGE && !memcmp(&c, &old, sizeof c));
  f[1].value = 1;
  f[2].value = 2;
  CHECK(ar_config(f, 4, &c) == AR_RANGE);
  f[1].value = 2;
  CHECK(ar_config(f, 4, &c) == AR_OK);
  unsigned char a[8], b[8];
  CHECK(ar_identity(c, a) == AR_OK);
  ArField reordered[] = {{4, 20}, {128, 999}, {3, 2}, {1, 1}, {2, 2}};
  CHECK(ar_config(reordered, 5, &c) == AR_OK && ar_identity(c, b) == AR_OK &&
        !memcmp(a, b, 8));
  reordered[1].key = 5;
  old = c;
  CHECK(ar_config(reordered, 5, &c) == AR_INVALID &&
        !memcmp(&c, &old, sizeof c));
  reordered[1].key = 4;
  CHECK(ar_config(reordered, 5, &c) == AR_INVALID);
  printf("PASS config capacity=%d feature=%d sizeof=%zu "
         "canonical=0102020000000014\n",
         AR_CAP, AR_FEATURE, sizeof(ArConfig));
}
