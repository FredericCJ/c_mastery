#include "diagnostics.h"
unsigned ae_threshold = 2;
#if AE_DEV
unsigned ae_developer_buffer[4];
static unsigned developer_count;
#endif
#if AE_INTEGRATOR
unsigned ae_integrator_buffer[4];
static unsigned integrator_count;
#endif
void ae_developer_emit(unsigned value) {
#if AE_DEV
  ae_developer_buffer[developer_count % 4] = value;
  if (developer_count < 65535)
    ++developer_count;
#else
  (void)value;
#endif
}
void ae_integrator_emit(unsigned severity, unsigned value) {
#if AE_INTEGRATOR
  if (severity >= ae_threshold) {
    ae_integrator_buffer[integrator_count % 4] = value;
    if (integrator_count < 65535)
      ++integrator_count;
  }
#else
  (void)severity;
  (void)value;
#endif
}
enum ae_code ae_production_capacity(unsigned n) { return n > 3 ? AE_INVALID : AE_OK; }
unsigned ae_diagnostic_events(unsigned category) {
#if AE_DEV
  if (category == 0)
    return developer_count;
#endif
#if AE_INTEGRATOR
  if (category == 1)
    return integrator_count;
#endif
  (void)category;
  return 0;
}
