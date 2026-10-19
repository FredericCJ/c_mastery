#include "public_error.h"
#include <limits.h>
#ifndef AE_DETAIL_VERSION
#define AE_DETAIL_VERSION 1
#endif
#if AE_DETAIL_VERSION == 1
typedef struct {
  int cause;
  unsigned backend, operation, stage;
} Detail;
#else
typedef struct {
  unsigned char bytes[8];
  unsigned long noise[5];
} Detail;
#endif
extern AeCode ae_map(unsigned, int);
AeCode ae_capture(unsigned backend, int cause, unsigned op, unsigned stage,
                  AePublic *out, unsigned char snapshot[8]) {
  if (!out || !snapshot || backend > 1 || cause < 0 || cause > 65535 || !op ||
      op > 31 || stage > 3)
    return AE_INVALID;
  unsigned char bytes[8] = {1,
                            (unsigned char)backend,
                            (unsigned char)op,
                            (unsigned char)stage,
                            (unsigned char)((unsigned)cause & 255u),
                            (unsigned char)((unsigned)cause >> 8),
                            0,
                            0};
#if AE_DETAIL_VERSION == 1
  Detail detail = {cause, backend, op, stage};
  AePublic result = {(uint32_t)ae_map(detail.backend, detail.cause),
                     detail.operation};
#else
  Detail detail = {0};
  for (unsigned i = 0; i < 8; ++i)
    detail.bytes[i] = bytes[i];
  detail.noise[4] = 99;
  AePublic result = {
      (uint32_t)ae_map(detail.bytes[1], (int)((unsigned)detail.bytes[4] +
                                              256u * detail.bytes[5])),
      detail.bytes[2]};
#endif
  for (unsigned i = 0; i < 8; ++i)
    snapshot[i] = bytes[i];
  *out = result;
  return AE_OK;
}
AeCode ae_decode(const unsigned char *b, size_t n, unsigned *backend,
                 int *cause, unsigned *op, unsigned *stage) {
  if (!b || n != 8 || !backend || !cause || !op || !stage || b[0] != 1 ||
      b[1] > 1 || !b[2] || b[2] > 31 || b[3] > 3 || b[6] || b[7])
    return AE_INVALID;
  unsigned value = (unsigned)b[4] + 256u * b[5];
  if (value > (unsigned)INT_MAX)
    return AE_FULL;
  *backend = b[1];
  *cause = (int)value;
  *op = b[2];
  *stage = b[3];
  return AE_OK;
}
