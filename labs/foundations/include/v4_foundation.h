#ifndef CMASTERY_V4_FOUNDATION_H
#define CMASTERY_V4_FOUNDATION_H
#include <stdint.h>
typedef struct {
  uint32_t available, active;
} V4Admission;
/* Admission is all-or-nothing; release cannot release an unowned ticket. */
int v4_admit(V4Admission *, uint32_t);
int v4_release(V4Admission *, uint32_t);
typedef struct {
  uint64_t wall_ns, cpu_ns, read_bytes, write_accepted, write_completed, io_ops;
  uint32_t cycles;
} V4Sample;
typedef struct {
  V4Sample delta;
  unsigned cycle_wrap;
} V4Measurement;
/* Counter difference only. Clock calibration and one-wrap premise are external
   evidence. Accepted output and actual completion remain distinct measures. */
int v4_measure(const V4Sample *, const V4Sample *, V4Measurement *);
#endif
