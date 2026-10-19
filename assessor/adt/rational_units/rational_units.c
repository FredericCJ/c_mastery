#include "rational_units.h"
#ifndef ADQ_MUTATION
#define ADQ_MUTATION 0
#endif
AdqStatus adq_scale(int64_t value, uint32_t numerator, uint32_t denominator,
                    AdqResult *out) {
  if (!out || !numerator || !denominator)
    return ADQ_INVALID;
  unsigned negative = value < 0;
  uint64_t magnitude =
      negative ? UINT64_C(0) - (uint64_t)value : (uint64_t)value;
  uint64_t limit = (uint64_t)INT64_MAX + (negative && ADQ_MUTATION != 3);
  uint64_t whole = magnitude / denominator, remainder = magnitude % denominator;
  if (whole > limit / numerator)
    return ADQ_RANGE;
  uint64_t product = remainder * numerator;
  uint64_t extra = product / denominator;
  uint64_t result = whole * numerator;
  if (extra > limit - result)
    return ADQ_RANGE;
  if (ADQ_MUTATION != 1)
    result += extra;
  int64_t signed_result;
  if (negative)
    signed_result =
        result == (uint64_t)INT64_MAX + 1 ? INT64_MIN : -(int64_t)result;
  else
    signed_result = (int64_t)result;
  *out = (AdqResult){signed_result,
                     ADQ_MUTATION == 2 ? 0 : product % denominator != 0};
  return ADQ_OK;
}
AdqStatus adq_to_ticks(AdqMicroseconds value, AdqPeriod period,
                       AdqResult *out) {
  return adq_scale(
      value.value, ADQ_MUTATION == 4 ? period.numerator_us : period.denominator,
      ADQ_MUTATION == 4 ? period.denominator : period.numerator_us, out);
}
AdqStatus adq_to_microseconds(AdqTicks value, AdqPeriod period,
                              AdqResult *out) {
  return adq_scale(value.value, period.numerator_us, period.denominator, out);
}
