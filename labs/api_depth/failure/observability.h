#ifndef CMASTERY_FAILURE_OBSERVABILITY_H
#define CMASTERY_FAILURE_OBSERVABILITY_H
#include "public_error.h"
struct ae_record {
  unsigned severity, unit, width, generation, length, tick;
  unsigned char payload[4];
};
enum ae_code ae_record_encode(const struct ae_record *, unsigned char *, size_t);
enum ae_code ae_record_decode(const unsigned char *, size_t, struct ae_record *);
/* -1 before,0 equal,1 after; half-cycle ambiguity returns AE_AMBIGUOUS. */
enum ae_code ae_tick_order(unsigned a, unsigned b, unsigned width, int *order);
struct ae_rate {
  unsigned tokens, remainder;
};
enum ae_code ae_rate_refill(struct ae_rate *, unsigned elapsed);
struct ae_log {
  struct ae_record records[3];
  unsigned count, dropped, rejected, limited, stalls;
  struct ae_rate rate;
};
enum ae_log_policy { AE_DROP_OLDEST, AE_PROTECT_HIGH };
enum ae_code ae_log(struct ae_log *, const struct ae_record *, enum ae_log_policy);
typedef int (*ae_sink_fn)(void *, const unsigned char *, size_t);
enum ae_code ae_log_drain(struct ae_log *, ae_sink_fn, void *);
struct ae_reporter {
  unsigned busy, stored, lost;
  struct ae_record fallback[2];
  unsigned fallback_available;
};
enum ae_code ae_report(struct ae_reporter *, const struct ae_record *, ae_sink_fn, void *);
#endif
