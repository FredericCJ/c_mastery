#ifndef CMASTERY_V4_FOUNDATION_H
#define CMASTERY_V4_FOUNDATION_H
#include <stdint.h>
struct v4_admission {
  uint32_t available, active;
};
/* Admission is all-or-nothing; release cannot release an unowned ticket. */
int v4_admit(struct v4_admission *, uint32_t);
int v4_release(struct v4_admission *, uint32_t);
struct v4_sample {
  uint64_t wall_ns, cpu_ns, read_bytes, write_accepted, write_completed, io_ops;
  uint32_t cycles;
};
struct v4_measurement {
  struct v4_sample delta;
  unsigned cycle_wrap;
};
/* Counter difference only. Clock calibration and one-wrap premise are external
   evidence. Accepted output and actual completion remain distinct measures. */
int v4_measure(const struct v4_sample *, const struct v4_sample *, struct v4_measurement *);
#endif
