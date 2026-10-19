#include "adt.h"
#include <inttypes.h>
#include <stdio.h>
/* Separate translation unit, no LTO: actual constructor invocation per value.
 */
int main(void) {
  uint64_t accepted = 0, rejected = 0, checksum = 0;
  for (uint64_t raw = 0; raw < UINT64_C(4294967296); ++raw) {
    ADFlags out = {UINT32_C(0xdeadbeef)};
    int ok = ad_flags((uint32_t)raw, &out);
    int expected = raw < 8;
    if (ok != expected ||
        out.bits != (expected ? (uint32_t)raw : UINT32_C(0xdeadbeef))) {
      fprintf(stderr,
              "ADO_CHECK exhaustive raw=%" PRIu64 " status=%d output=%" PRIu32
              "\n",
              raw, ok, out.bits);
      return 1;
    }
    if (ok) {
      ++accepted;
      checksum += out.bits;
    } else
      ++rejected;
  }
  printf("{\"values\":4294967296,\"accepted\":%" PRIu64 ",\"rejected\":%" PRIu64
         ",\"checksum\":%" PRIu64 ",\"unchanged_rejections\":true}\n",
         accepted, rejected, checksum);
  return 0;
}
