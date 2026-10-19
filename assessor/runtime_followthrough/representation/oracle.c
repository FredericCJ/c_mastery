#include "representation.h"
#include <inttypes.h>
#include <stdio.h>
int main(void) {
  unsigned op, ta, tb, status, tag, count = 0;
  int32_t a, b, result;
  while (scanf("%u %u %" SCNd32 " %u %" SCNd32 " %u %u %" SCNd32, &op, &ta, &a,
               &tb, &b, &status, &tag, &result) == 8) {
    RvValue out = {RV_REF, 7, 19};
    int got = rv_arithmetic(op, (RvValue){ta, a, 0}, (RvValue){tb, b, 0}, &out);
    if (got != (int)status ||
        (got == RV_OK
             ? (out.tag != tag || out.payload != result || out.generation)
             : (out.tag != RV_REF || out.payload != 7 ||
                out.generation != 19))) {
      printf("RV_ORACLE_FAIL case=%u status=%d/%u tag=%u/%u payload=%" PRId32
             "/%" PRId32 "\n",
             count, got, status, out.tag, tag, out.payload, result);
      return 1;
    }
    if (++count > 50000)
      return 2;
  }
  if (!feof(stdin) || count < 10000)
    return 2;
  printf("RV_ORACLE_PASS cases=%u rational_scale_semantics\n", count);
  return 0;
}
