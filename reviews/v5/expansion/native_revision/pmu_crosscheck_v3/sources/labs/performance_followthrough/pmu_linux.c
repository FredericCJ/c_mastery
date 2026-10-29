#define _GNU_SOURCE
#include "pmu.h"
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

static int event_open(void *context, PFPmuEvent event, int leader) {
    struct perf_event_attr attr = {0};
    (void)context;
    attr.type = event.type;
    attr.size = sizeof attr;
    attr.config = event.config;
    attr.disabled = leader == -1;
    attr.exclude_kernel = 1;
    attr.exclude_hv = 1;
    attr.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_TOTAL_TIME_ENABLED |
                       PERF_FORMAT_TOTAL_TIME_RUNNING | PERF_FORMAT_ID;
    return (int)syscall(SYS_perf_event_open, &attr, 0, -1, leader,
                        PERF_FLAG_FD_CLOEXEC);
}
static int control(void *context, int fd, enum PFPmuControl op, uint64_t *id) {
    (void)context;
    if (op == PF_PMU_ID) return ioctl(fd, PERF_EVENT_IOC_ID, id);
    unsigned long request = op == PF_PMU_RESET ? PERF_EVENT_IOC_RESET :
                            op == PF_PMU_ENABLE ? PERF_EVENT_IOC_ENABLE :
                                                  PERF_EVENT_IOC_DISABLE;
    return ioctl(fd, request, PERF_IOC_FLAG_GROUP);
}
static ptrdiff_t event_read(void *context, int fd, void *bytes, size_t n) {
    (void)context;
    return read(fd, bytes, n);
}
static int event_close(void *context, int fd) {
    (void)context;
    return close(fd);
}
PFPmuOS pf_pmu_linux(void) {
    return (PFPmuOS){NULL, event_open, control, event_read, event_close};
}
const char *pf_pmu_state_name(PFPmuState state) {
    static const char *const names[] = {
        "empty", "ready", "active", "measured", "denied", "unavailable",
        "io-error", "never-scheduled", "malformed", "invalid", "unimplemented"
    };
    return (unsigned)state <= PF_PMU_UNIMPLEMENTED ? names[state] : "invalid";
}
