#include "session_v2.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "octet representation requires eight-bit bytes");
int ac_typed_v1_set(struct ac_typed_v1 *s, uint32_t id, uint32_t n) {
  if (!s)
    return 1;
  s->private_words[0] = id;
  s->private_words[1] = n;
  return 0;
}
int ac_typed_v1_get(const struct ac_typed_v1 *s, struct ac_exposed_v1 *out) {
  if (!s || !out)
    return 1;
  *out = (struct ac_exposed_v1){s->private_words[0], s->private_words[1]};
  return 0;
}
int ac_serialized_set(unsigned char out[12], uint32_t id, uint64_t n) {
  if (!out)
    return 1;
  for (unsigned i = 0; i < 4; ++i)
    out[i] = (unsigned char)(id >> (8 * i));
  uint32_t low=(uint32_t)n,high=(uint32_t)(n>>32);
  for (unsigned i = 0; i < 4; ++i) {
#ifdef AC_MUTANT_ENDIAN
    out[i+4]=(unsigned char)(low>>(8*(3-i)));
#else
    out[i+4]=(unsigned char)(low>>(8*i));
#endif
    out[i+8]=(unsigned char)(high>>(8*i));
  }
  return 0;
}
int ac_serialized_get(const unsigned char in[12], struct ac_exposed_v2 *out) {
  if (!in || !out)
    return 1;
  uint32_t id = 0;
  uint32_t low=0,high=0;
  for (unsigned i = 0; i < 4; ++i)
    id |= (uint32_t)in[i] << (8 * i);
  for (unsigned i = 0; i < 4; ++i) {
    low|=(uint32_t)in[i+4]<<(8*i);high|=(uint32_t)in[i+8]<<(8*i);
  }
  *out = (struct ac_exposed_v2){id, ((uint64_t)high<<32)|low};
  return 0;
}
int ac_typed_v2_set(struct ac_typed_v2 *s, uint32_t id, uint64_t n) {
  if (!s)
    return 1;
  int r = ac_serialized_set(s->private_octets, id, n);
  for (unsigned i = 12; i < 16; ++i)
    s->private_octets[i] = 0;
  return r;
}
int ac_typed_v2_get(const struct ac_typed_v2 *s, struct ac_exposed_v2 *out) {
  if (!s)
    return 1;
  return ac_serialized_get(s->private_octets, out);
}
