#ifndef CMASTERY_FAILURE_PUBLIC_ERROR_H
#define CMASTERY_FAILURE_PUBLIC_ERROR_H
#include <stddef.h>
#include <stdint.h>
enum ae_code {
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
};
struct ae_public {
  uint32_t code, operation;
};
/* Diagnostic snapshot is an explicit versioned encoding, never a borrowed
 * message. */
enum ae_code ae_capture(unsigned backend, int cause, unsigned operation,
                  unsigned stage, struct ae_public *, unsigned char snapshot[8]);
enum ae_code ae_decode(const unsigned char *, size_t, unsigned *backend, int *cause,
                 unsigned *operation, unsigned *stage);
#endif
