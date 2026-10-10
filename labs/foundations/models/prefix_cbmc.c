#include "foundation.h"
#include <assert.h>
#include <limits.h>
extern int32_t nondet_int32_t(void);
extern unsigned nondet_unsigned(void);
void __CPROVER_assume(_Bool);
int main(void) {
  int32_t input[4], output[4] = {77, 77, 77, 77};
  int64_t sum = 0;
  unsigned n = nondet_unsigned();
  int valid = 1;
  __CPROVER_assume(n <= 4);
  for (unsigned i = 0; i < n; i++)
    input[i] = nondet_int32_t();
  for (unsigned i = 0; i < n; i++) {
    sum += input[i];
    if (sum < INT32_MIN || sum > INT32_MAX)
      valid = 0;
  }
  FStatus status = f_prefix(input, n, output, 4);
  if (!valid) {
    assert(status == F_RANGE);
    for (unsigned i = 0; i < 4; i++)
      assert(output[i] == 77);
  } else {
    assert(status == F_OK);
    sum = 0;
    for (unsigned i = 0; i < n; i++) {
      sum += input[i];
      assert(output[i] == sum);
    }
  }
  return 0;
}
