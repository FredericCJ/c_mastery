#ifndef CMASTERY_AD_RATIONAL_UNITS_H
#define CMASTERY_AD_RATIONAL_UNITS_H
#include "adt.h"
typedef struct {
  int64_t value;
} AdqMicroseconds;
typedef struct {
  int64_t value;
} AdqTicks;
typedef struct {
  uint32_t numerator_us, denominator;
} AdqPeriod;
typedef struct {
  int64_t value;
  unsigned inexact;
} AdqResult;
typedef enum { ADQ_OK, ADQ_INVALID, ADQ_RANGE, ADQ_UNIMPLEMENTED } AdqStatus;
AdqStatus adq_scale(int64_t, uint32_t, uint32_t, AdqResult *);
AdqStatus adq_to_ticks(AdqMicroseconds, AdqPeriod, AdqResult *);
AdqStatus adq_to_microseconds(AdqTicks, AdqPeriod, AdqResult *);
#endif
