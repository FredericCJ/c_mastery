#ifndef CMASTERY_SESSION_V1_H
#define CMASTERY_SESSION_V1_H
#include <stddef.h>
#include <stdint.h>
typedef struct {
  uint32_t identity, count;
} AcExposedV1;
typedef struct {
  uint32_t private_words[2];
} AcTypedV1;
typedef struct AcOpaque AcOpaque;
typedef struct {
  void *context;
  void *(*allocate)(void *, size_t, size_t);
  void (*release)(void *, void *);
} AcAllocate;
int ac_typed_v1_set(AcTypedV1 *, uint32_t identity, uint32_t count);
int ac_typed_v1_get(const AcTypedV1 *, AcExposedV1 *);
int ac_opaque_create(uint32_t identity, uint32_t count, AcAllocate,
                     AcOpaque **);
int ac_opaque_get(const AcOpaque *, AcExposedV1 *);
void ac_opaque_destroy(AcOpaque *);
size_t ac_opaque_size(void);
size_t ac_opaque_alignment(void);
#endif
