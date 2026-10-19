#include "test.h"
static int ref(int a, int b, unsigned q) {
  int d = 1 << q, p = a * b;
  int t = p / d, r = p % d;
  if (r >= d / 2)
    t++;
  if (r <= -d / 2)
    t--;
  if (t >= d)
    t = d - 1;
  if (t < -d)
    t = -d;
  return t;
}
int main(void) {
  int16_t out;
  for (int a = -128; a < 128; a++)
    for (int b = -128; b < 128; b++) {
      CHECK(nd_qmul((int16_t)a, (int16_t)b, 7, &out) == ND_OK);
      CHECK(out == ref(a, b, 7));
    }
  for (int a = -2048; a < 2048; a += 17)
    for (int b = -2048; b < 2048; b += 19) {
      CHECK(nd_qmul((int16_t)a, (int16_t)b, 11, &out) == ND_OK &&
            out == ref(a, b, 11));
    }
  int16_t a[32], b[32];
  for (size_t i = 0; i < 32; i++) {
    a[i] = -128;
    b[i] = -128;
  }
  CHECK(nd_qmac(a, b, 32, 7, &out) == ND_OK && out == 127);
  for (size_t i = 0; i < 32; i++) {
    a[i] = -2048;
    b[i] = 2047;
  }
  CHECK(nd_qmac(a, b, 32, 11, &out) == ND_OK && out == -2048);
  a[0] = a[1] = 1;
  b[0] = b[1] = 64;
  CHECK(nd_qmac(a, b, 2, 7, &out) == ND_OK && out == 1);
  int16_t p;
  CHECK(nd_qmul(1, 64, 7, &p) == ND_OK && p + p == 2);
  double values[] = {-2,        -1, -.5,         -1.0 / 256, 0,
                     1.0 / 256, .5, 127.0 / 128, 1,          2};
  int expected[] = {-128, -128, -64, -1, 0, 1, 64, 127, 127, 127};
  for (size_t i = 0; i < 10; i++)
    CHECK(nd_quantize(values[i], 7, &out) == ND_OK && out == expected[i]);
  out = 42;
  CHECK(nd_quantize(NAN, 7, &out) == ND_BAD && out == 42);
  CHECK(nd_qmul(128, 0, 7, &out) == ND_BAD && out == 42);
  CHECK(nd_qmac(a, b, 33, 7, &out) == ND_BAD && out == 42);
  puts("PASS exhaustive Q7, Q11 lattice, final-round MAC and clipping");
}
