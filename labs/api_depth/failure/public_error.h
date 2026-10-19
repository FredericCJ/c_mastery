#ifndef CMASTERY_FAILURE_PUBLIC_ERROR_H
#define CMASTERY_FAILURE_PUBLIC_ERROR_H
#include <stddef.h>
#include <stdint.h>
typedef enum {
  AE_OK,
  AE_INVALID,
  AE_CONTRACT,
  AE_EXHAUSTED,
  AE_DEVICE,
  AE_TIMEOUT,
  AE_UNKNOWN,
  AE_AGAIN,
  AE_AMBIGUOUS,
  AE_CONFLICT,
  AE_FULL,
  AE_UNIMPLEMENTED
} AeCode;
typedef struct {
  uint32_t code, operation;
} AePublic;
/* Diagnostic snapshot is an explicit versioned encoding, never a borrowed
 * message. */
AeCode ae_capture(unsigned backend, int cause, unsigned operation,
                  unsigned stage, AePublic *, unsigned char snapshot[8]);
AeCode ae_decode(const unsigned char *, size_t, unsigned *backend, int *cause,
                 unsigned *operation, unsigned *stage);
#endif
