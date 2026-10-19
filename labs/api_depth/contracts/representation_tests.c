#include "session_v2.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "REPRESENTATION-CHECK:%s line%d\n", #x, __LINE__);       \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  AcTypedV1 old;
  AcExposedV1 v1;
  CHECK(!ac_typed_v1_set(&old, 17, UINT32_MAX) && !ac_typed_v1_get(&old, &v1));
  CHECK(v1.identity == 17 && v1.count == UINT32_MAX);
  AcTypedV2 next;
  AcExposedV2 v2;
  CHECK(!ac_typed_v2_set(&next, 17, UINT64_MAX) &&
        !ac_typed_v2_get(&next, &v2));
  CHECK(v2.identity == 17 && v2.count == UINT64_MAX);
  unsigned char bytes[12];
  CHECK(!ac_serialized_set(bytes, 0x12345678u, UINT64_C(0x0102030405060708)));
  const unsigned char expected[12] = {0x78, 0x56, 0x34, 0x12, 8, 7,
                                      6,    5,    4,    3,    2, 1};
  CHECK(!memcmp(bytes, expected, 12) && !ac_serialized_get(bytes, &v2));
  CHECK(v2.identity == 0x12345678u && v2.count == UINT64_C(0x0102030405060708));
  CHECK(ac_typed_v2_get(NULL, &v2) == 1 && v2.identity == 0x12345678u);
  printf("REPRESENTATION-PASS:exposed_v1=%zu/%zu exposed_v2=%zu/%zu "
         "typed_v1=%zu/%zu typed_v2=%zu/%zu serialized=12\n",
         sizeof(AcExposedV1), _Alignof(AcExposedV1), sizeof(AcExposedV2),
         _Alignof(AcExposedV2), sizeof(AcTypedV1), _Alignof(AcTypedV1),
         sizeof(AcTypedV2), _Alignof(AcTypedV2));
  return 0;
}
