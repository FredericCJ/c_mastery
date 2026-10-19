#ifndef CMASTERY_FAILURE_TAXONOMY_H
#define CMASTERY_FAILURE_TAXONOMY_H
#include "generic.h"
#include "public_error.h"
typedef enum {
  AE_RECOVERABLE,
  AE_RETRYABLE,
  AE_TERMINAL,
  AE_DEGRADED
} AeOutcome;
typedef struct {
  unsigned stage;
  int cause;
} AeSecondary;
typedef struct {
  AePublic result;
  unsigned stage, backend;
  int first;
  AeSecondary secondary[2];
  unsigned stored, dropped, saturated;
} AeError;
AeCode ae_map(unsigned, int);
AeOutcome ae_outcome(AeCode);
AeCode ae_first(AeError *, AeCode, unsigned operation, unsigned stage,
                unsigned backend, int cause);
AeCode ae_secondary(AeError *, unsigned stage, int cause);
AeCode ae_boundary(const unsigned char *, size_t, unsigned internal_count,
                   unsigned allocation_failed, unsigned device_failed,
                   unsigned *out);
typedef struct {
  unsigned value, admitted;
} AeDigit;
AeCode ae_admit_digit(const unsigned char *, size_t, AeDigit *);
AeCode ae_trusted_digit(AeDigit, unsigned *);
typedef struct {
  unsigned id, delta;
} AeEntry;
typedef struct {
  AeEntry history[2];
  unsigned count, next, value, effects;
} AeCounter;
typedef enum { AE_SEND, AE_BUSY, AE_FAIL_BEFORE, AE_LOSE_ACK } AeTransportMode;
AeCode ae_submit(AeCounter *, unsigned id, unsigned delta, AeTransportMode);
typedef struct {
  AeCode code;
  size_t accepted;
} AeWriteResult;
AeCode ae_write_result(size_t offered, size_t accepted, AeCode,
                       AeWriteResult *);
typedef struct {
  unsigned found, value;
} AeLookup;
AeLookup ae_lookup(unsigned key);
typedef struct {
  unsigned field;
  size_t offset;
  int cause;
} AeFieldError;
typedef struct {
  unsigned count;
  int values[4];
  AeFieldError errors[4];
} AeFields;
AeCode ae_fields(const AvText *, size_t, AeFields *);
#endif
