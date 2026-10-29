#ifndef CF_MEASURE_H
#define CF_MEASURE_H
/* Host POSIX measurement adapters, outside the freestanding core. kind0=SPSC,
 * 1=frozen AP,2=mutex/condition queue,3=mutex with try admission.
 * affinity0=unrestricted current allowed CPUs,1=all workers on one allowed
 * CPU,2=round-robin on two allowed CPUs. All threads joined before caller-owned
 * state or output name is released. */
int cf_queue_measure(unsigned kind, unsigned pairs, unsigned bytes,
                     unsigned affinity, unsigned pause, unsigned total,
                     const char *csv);
int cf_rw_measure(unsigned kind, unsigned pause, unsigned writes,
                  const char *csv);
int cf_pool_measure(unsigned policy, unsigned iterations, const char *csv);
#endif
