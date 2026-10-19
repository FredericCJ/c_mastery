#ifndef FLOAT_CONTRACT_H
#define FLOAT_CONTRACT_H
#include <stddef.h>
#include <stdint.h>
#include <fenv.h>
typedef enum { FF_OK, FF_RANGE, FF_MODEL, FF_ENV, FF_RESTORE } FfStatus;
typedef enum { FF_NAIVE, FF_KAHAN, FF_NEUMAIER } FfMethod;
typedef struct { int modes[4]; double values[4]; size_t count; } FfRound;
/* Injection seam: functions retain the corresponding fenv function contract. */
typedef struct {
    int (*save)(fenv_t *);
    int (*hold)(fenv_t *);
    int (*mode)(int);
    int (*restore)(const fenv_t *);
} FfEnvOps;
FfStatus ff_sum(const double *, size_t, FfMethod, double *);
FfStatus ff_dot(const double *, const double *, size_t, double *);
FfStatus ff_compare(double, double, double, double, int, int *);
FfStatus ff_ulps(double, double, uint64_t *);
FfStatus ff_probe_ops(double, double, FfRound *, const FfEnvOps *);
FfStatus ff_round_probe(double, double, FfRound *);
#endif
