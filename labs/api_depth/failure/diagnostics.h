#ifndef CMASTERY_FAILURE_DIAGNOSTICS_H
#define CMASTERY_FAILURE_DIAGNOSTICS_H
#include "public_error.h"
#ifndef AE_DEV
#define AE_DEV 0
#endif
#ifndef AE_INTEGRATOR
#define AE_INTEGRATOR 0
#endif
extern unsigned ae_threshold;
void ae_developer_emit(unsigned);
void ae_integrator_emit(unsigned, unsigned);
enum ae_code ae_production_capacity(unsigned);
unsigned ae_diagnostic_events(unsigned category);
#if AE_DEV
#define AE_TRACE(value) ae_developer_emit(value)
#elif defined(AE_MUTANT_DISABLED_EVAL)
#define AE_TRACE(value) ((void)(value))
#else
#define AE_TRACE(value) ((void)0)
#endif
#if AE_INTEGRATOR
#define AE_DIAG(severity, value)                                               \
  do {                                                                         \
    unsigned ae_level_ = (severity);                                           \
    if (ae_level_ >= ae_threshold)                                             \
      ae_integrator_emit(ae_level_, (value));                                  \
  } while (0)
#else
#define AE_DIAG(severity, value) ((void)0)
#endif
#endif
