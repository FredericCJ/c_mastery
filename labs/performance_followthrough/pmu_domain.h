/* Linux adapter for generic events on hybrid core PMUs (perf_event.h UAPI).
 * native.py chooses the type from sysfs and pins the collector to that CPU.
 * With no selection, direct collector invocations use the kernel default. */
#ifndef PF_PMU_DOMAIN_H
#define PF_PMU_DOMAIN_H

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

static inline int pf_generic_event_config(uint64_t event, uint64_t *config) {
    const char *digits = getenv("PF_PMU_DOMAIN_TYPE");
    uint64_t type = 0;
    if (digits != NULL) {
        if (!*digits) {
            errno = EINVAL;
            return -1;
        }
        for (; *digits; ++digits) {
            if (*digits < '0' || *digits > '9' || type > (UINT32_MAX - (unsigned)(*digits - '0')) / 10u) {
                errno = EINVAL;
                return -1;
            }
            type = type * 10u + (unsigned)(*digits - '0');
        }
    }
    *config = (type << 32) | event;
    return 0;
}
#endif
