#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "REPRESENTATION-CHECK:%d:%s\n", __LINE__, #x);           \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct padded {
  char tag;
  int value;
};
int main(void) {
  const int values[] = {0, 1, -1, INT_MIN, INT_MAX};
  for (unsigned i = 0; i < 5; ++i) {
    unsigned char representation[sizeof(int)];
    int result = 0;
    CHECK(memcpy(representation, &values[i], sizeof(int)) == representation);
    CHECK(memcpy(&result, representation, sizeof result) == &result &&
          result == values[i]);
  }
  unsigned char wire[] = {0x12, 0x34};
  uint32_t admitted = ((uint32_t)wire[0] << 8) | wire[1];
  CHECK(admitted == 0x1234); /* No arbitrary-wire-to-int pointer cast. */
  struct padded x = {'A', 42}, y = {'A', 42};
  CHECK(offsetof(struct padded, value) > sizeof(char));
  unsigned char *xb = (unsigned char *)&x, *yb = (unsigned char *)&y;
  /* Write every known padding byte after member stores; do not rely on compiler
     padding initialization or compare uninitialized padding. */
  for (size_t i = 0; i < sizeof x; ++i)
    if (i >= sizeof(char) &&
        (i < offsetof(struct padded, value) ||
         i >= offsetof(struct padded, value) + sizeof(int))) {
      xb[i] = 0x11;
      yb[i] = 0x22;
    }
  CHECK(x.tag == y.tag && x.value == y.value && memcmp(&x, &y, sizeof x) != 0);
  printf("PADDING size=%zu value-offset=%zu\n", sizeof x,
         offsetof(struct padded, value));
  const char *inputs[] = {"abc", "abcdef"};
  for (unsigned k = 0; k < 2; ++k)
    for (size_t cap = 3; cap <= 4; ++cap) {
      char copy[8], record[8], format[8];
      memset(copy, '#', 8);
      memset(record, '#', 8);
      memset(format, '#', 8);
      size_t len = strlen(inputs[k]);
      if (len + 1 <= cap)
        CHECK(
            strcpy(copy, inputs[k]) == copy &&
            !strcmp(
                copy,
                inputs[k])); /* Otherwise analysis-only: strcpy has no bound. */
      CHECK(strncpy(record, inputs[k], cap) == record);
      for (size_t i = 0; i < cap; ++i)
        CHECK(record[i] == (i < len ? inputs[k][i] : 0));
      CHECK(record[cap] == '#');
      int n = snprintf(format, cap, "%s", inputs[k]);
      CHECK(n == (int)len && format[cap - 1] == 0);
      for (size_t i = 0; i < cap - 1; ++i)
        CHECK(format[i] == inputs[k][i]);
      CHECK(format[cap] == '#');
    }
  unsigned char bytes[] = {0, 1, 2, 3, 4};
  CHECK(memmove(bytes + 1, bytes, 4) == bytes + 1 &&
        !memcmp(bytes, (unsigned char[]){0, 0, 1, 2, 3}, 5));
  CHECK(memcpy(bytes, wire, 2) == bytes && bytes[0] == 0x12 &&
        bytes[1] == 0x34);
  CHECK(memcpy(bytes, wire, 0) == bytes);
  puts("REPRESENTATION-PASS:valid-roundtrip:wire:padding:copy-contracts");
  return 0;
}
