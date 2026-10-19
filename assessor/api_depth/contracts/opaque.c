#include "session_v1.h"
#ifndef AC_INTERNAL_WIDE
#define AC_INTERNAL_WIDE 0
#endif
struct AcOpaque {
  uint32_t identity;
#if AC_INTERNAL_WIDE
  uint64_t count;
#else
  uint32_t count;
#endif
  AcAllocate allocator;
};
int ac_opaque_create(uint32_t id, uint32_t n, AcAllocate a, AcOpaque **out) {
  if (!out || !a.allocate || !a.release)
    return 1;
#ifdef AC_MUTANT_OUTPUT
  *out=NULL;
#endif
  AcOpaque *s = a.allocate(a.context, sizeof *s, _Alignof(AcOpaque));
  if (!s)
    return 2;
  s->identity = id;
  s->count = n;
  s->allocator = a;
  *out = s;
  return 0;
}
int ac_opaque_get(const AcOpaque *s, AcExposedV1 *out) {
  if (!s || !out)
    return 1;
  *out = (AcExposedV1){s->identity, (uint32_t)s->count};
  return 0;
}
void ac_opaque_destroy(AcOpaque *s) {
  if (s) {
    AcAllocate a = s->allocator;
    a.release(a.context, s);
  }
}
size_t ac_opaque_size(void) { return sizeof(AcOpaque); }
size_t ac_opaque_alignment(void) { return _Alignof(AcOpaque); }
