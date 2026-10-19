#ifndef CMASTERY_FAILURE_OBSERVABILITY_H
#define CMASTERY_FAILURE_OBSERVABILITY_H
#include "public_error.h"
typedef struct {
  unsigned severity, unit, width, generation, length, tick;
  unsigned char payload[4];
} AeRecord;
AeCode ae_record_encode(const AeRecord *, unsigned char *, size_t);
AeCode ae_record_decode(const unsigned char *, size_t, AeRecord *);
/* -1 before,0 equal,1 after; half-cycle ambiguity returns AE_AMBIGUOUS. */
AeCode ae_tick_order(unsigned a, unsigned b, unsigned width, int *order);
typedef struct {
  unsigned tokens, remainder;
} AeRate;
AeCode ae_rate_refill(AeRate *, unsigned elapsed);
typedef struct {
  AeRecord records[3];
  unsigned count, dropped, rejected, limited, stalls;
  AeRate rate;
} AeLog;
typedef enum { AE_DROP_OLDEST, AE_PROTECT_HIGH } AeLogPolicy;
AeCode ae_log(AeLog *, const AeRecord *, AeLogPolicy);
typedef int (*AeSink)(void *, const unsigned char *, size_t);
AeCode ae_log_drain(AeLog *, AeSink, void *);
typedef struct {
  unsigned busy, stored, lost;
  AeRecord fallback[2];
  unsigned fallback_available;
} AeReporter;
AeCode ae_report(AeReporter *, const AeRecord *, AeSink, void *);
#endif
