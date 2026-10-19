#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void *__real_malloc(size_t);
static unsigned active, requests;
void *__wrap_malloc(size_t n) {
  if (active)
    ++requests;
  return __real_malloc(n);
}
static int cmp(const void *a, const void *b) {
  const unsigned char *x = a, *y = b;
  return (x[0] > y[0]) - (x[0] < y[0]);
}
int main(void) {
  unsigned char records[96 * 64];
  const size_t widths[] = {32, 33, 64};
  for (unsigned w = 0; w < 3; ++w) {
    size_t width = widths[w];
    for (unsigned i = 0; i < 96; ++i) {
      memset(records + i * width, (int)i, width);
      records[i * width] = (unsigned char)((96 - i) % 7);
    }
    requests = 0;
    active = 1;
    qsort(records, 96, width, cmp);
    active = 0;
    if (requests)
      return 1;
    unsigned seen[96] = {0};
    for (unsigned i = 0; i < 96; ++i) {
      unsigned id = records[i * width + 1];
      if (id >= 96 || seen[id]++ || records[i * width] != (96 - id) % 7)
        return 2;
      for (size_t k = 1; k < width; ++k)
        if (records[i * width + k] != id)
          return 3;
      if (i && records[(i - 1) * width] > records[i * width])
        return 4;
    }
    printf(
        "width=%zu count=96 observed_malloc=0 public_order_permutation=pass\n",
        width);
  }
  puts("SORTING-PASS:musl1.2.5 three widths;hook scope bounded by "
       "source/static-object inspection");
  return 0;
}
