#ifndef CMASTERY_SESSION_V2_H
#define CMASTERY_SESSION_V2_H
#include "session_v1.h"
typedef struct {
  uint32_t identity;
  uint64_t count;
} AcExposedV2;
/* Actual typed storage. Operations never cast these bytes to another type. */
typedef struct {
  _Alignas(8) unsigned char private_octets[16];
} AcTypedV2;
int ac_typed_v2_set(AcTypedV2 *, uint32_t identity, uint64_t count);
int ac_typed_v2_get(const AcTypedV2 *, AcExposedV2 *);
int ac_serialized_set(unsigned char out[12], uint32_t identity, uint64_t count);
int ac_serialized_get(const unsigned char in[12], AcExposedV2 *);
#endif
