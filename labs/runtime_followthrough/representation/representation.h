#ifndef CMASTERY_RUNTIME_REPRESENTATION_H
#define CMASTERY_RUNTIME_REPRESENTATION_H
#include "cvm.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "CVM wire bytes require eight bits");
enum { RV_NIL, RV_INT, RV_BOOL, RV_REF, RV_Q16 };
enum {
  RV_OK,
  RV_ARGUMENT,
  RV_TYPE,
  RV_RANGE,
  RV_FORMAT,
  RV_VERSION,
  RV_REQUIRED,
  RV_UNIMPLEMENTED
};
enum { RV_ADD, RV_SUB, RV_MUL, RV_DIV };
struct rv_value {
  unsigned tag;
  int32_t payload;
  uint32_t generation;
};
/* Values are caller-owned, no allocation. Conversion rejects malformed tags,
 * BOOL other than0/1 and REF outside0..15 or generation0. Ignored scalar
 * generation/NIL payload fields are normalized to zero; no object dereference.
 */
int rv_import(struct cvm_value, struct rv_value *);
int rv_equal(struct rv_value, struct rv_value, int *);
/* INT/INT stays checked int32. Otherwise numeric operands must first fit Q16;
 * INT converts exactly by65536. Q16 MUL/DIV truncate toward zero after scaling.
 * EQ is tag-strict even when arithmetic admits mixed INT/Q16 operands.
 * Every refused operation leaves output unchanged. Output may alias an input.
 */
int rv_arithmetic(unsigned, struct rv_value, struct rv_value, struct rv_value *);
#define RV_IMAGE_MAX 1358u
struct rv_image {
  struct cvm_program program;
  uint8_t name[31], name_length, digest[32], has_digest, skipped;
};
/* Original RVE2 envelope: magic4,major1,flags1,base_lengthLE16,section_count1;
 * CVM1 bytes; count<=4 TLVs(type,flags,lengthLE16,data<=64). Known1=digest32,
 * 2=nonempty printable ASCII name<=31, each at most once. Flag1 means required;
 * unknown optional skips, unknown required rejects; other flag bits reject.
 * Encode writes known digest then name, no unknown sections. Decode is raw
 * container validation, never execution permission. Old CVM1 reader rejects it.
 * Live disjoint input/output extents; output/written unchanged on any failure.
 * Reported span larger than RV_IMAGE_MAX rejects before reading input. */
int rv_decode(const uint8_t *, size_t, struct rv_image *);
int rv_encode(const struct rv_image *, uint8_t *, size_t, size_t *);
#endif
