/* Supplied observation, not a solved learner algorithm. Compare prediction
 * first. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define A B
#define B 7
#define VALUE 7
#define STRING_RAW(x) #x
#define STRING_EXPANDED(x) STRING_RAW(x)
static int square(int x) { return x * x; }
struct sample {
  uint8_t flag;
  uint32_t count;
};
int main(void) {
  int value = 1;
  int result = square(++value);
  _Static_assert(sizeof(uint32_t) == 4, "selected uint32_t width");
  int type = _Generic(result, int: 1, default: 0);
  printf("rescanned=%d direct=%s expanded=%s value=%d result=%d int_type=%d "
         "offset=%zu size=%zu\n",
         A, STRING_RAW(VALUE), STRING_EXPANDED(VALUE), value, result, type,
         offsetof(struct sample, count), sizeof(struct sample));
  return A != 7 || strcmp(STRING_RAW(VALUE), "VALUE") ||
         strcmp(STRING_EXPANDED(VALUE), "7") || value != 2 || result != 4 ||
         type != 1;
}
