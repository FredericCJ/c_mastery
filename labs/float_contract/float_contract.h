#ifndef FLOAT_CONTRACT_H
#define FLOAT_CONTRACT_H
#include <stddef.h>
#include <stdint.h>
#include <fenv.h>
enum ff_status { FF_OK, FF_RANGE, FF_MODEL, FF_ENV, FF_RESTORE };
enum ff_method { FF_NAIVE, FF_KAHAN, FF_NEUMAIER };
struct ff_round { int modes[4]; double values[4]; size_t count; };
/* Injection seam: functions retain the corresponding fenv function contract. */
struct ff_env_ops {
    int (*save)(fenv_t *);
    int (*hold)(fenv_t *);
    int (*mode)(int);
    int (*restore)(const fenv_t *);
};
enum ff_status ff_sum(const double *, size_t, enum ff_method, double *);
enum ff_status ff_dot(const double *, const double *, size_t, double *);
enum ff_status ff_compare(double, double, double, double, int, int *);
enum ff_status ff_ulps(double, double, uint64_t *);
enum ff_status ff_probe_ops(double, double, struct ff_round *, const struct ff_env_ops *);
enum ff_status ff_round_probe(double, double, struct ff_round *);
#endif
