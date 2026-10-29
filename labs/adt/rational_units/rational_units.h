#ifndef CMASTERY_AD_RATIONAL_UNITS_H
#define CMASTERY_AD_RATIONAL_UNITS_H
#include "adt.h"
struct adq_microseconds {
  int64_t value;
};
struct adq_ticks {
  int64_t value;
};
struct adq_period {
  uint32_t numerator_us, denominator;
};
struct adq_result {
  int64_t value;
  unsigned inexact;
};
enum adq_status { ADQ_OK, ADQ_INVALID, ADQ_RANGE, ADQ_UNIMPLEMENTED };
enum adq_status adq_scale(int64_t, uint32_t, uint32_t, struct adq_result *);
enum adq_status adq_to_ticks(struct adq_microseconds, struct adq_period, struct adq_result *);
enum adq_status adq_to_microseconds(struct adq_ticks, struct adq_period, struct adq_result *);
#endif
