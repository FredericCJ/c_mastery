#ifndef CMASTERY_SESSION_V2_H
#define CMASTERY_SESSION_V2_H
#include "session_v1.h"
struct ac_exposed_v2 {
  uint32_t identity;
  uint64_t count;
};
/* Actual typed storage. Operations never cast these bytes to another type. */
struct ac_typed_v2 {
  _Alignas(8) unsigned char private_octets[16];
};
int ac_typed_v2_set(struct ac_typed_v2 *, uint32_t identity, uint64_t count);
int ac_typed_v2_get(const struct ac_typed_v2 *, struct ac_exposed_v2 *);
int ac_serialized_set(unsigned char out[12], uint32_t identity, uint64_t count);
int ac_serialized_get(const unsigned char in[12], struct ac_exposed_v2 *);
#endif
