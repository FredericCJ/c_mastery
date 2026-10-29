#include "pipeline.h"

enum ap_status ap_schedule(const struct ap_instruction *a, size_t b,
                           struct ap_config c, struct ap_schedule *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return AP_INVALID;
}

enum ap_status ap_ooo(const struct ap_instruction *a, size_t b,
                      const uint8_t c[8], const uint8_t d[16], unsigned e,
                      struct ap_ooo *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return AP_INVALID;
}
