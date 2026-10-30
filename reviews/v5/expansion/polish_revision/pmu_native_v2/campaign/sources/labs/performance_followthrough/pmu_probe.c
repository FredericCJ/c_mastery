/*
 * Supplied adapter: report what perf_event_open allows on this machine.
 *
 * Opens a strict two-event hardware group (instructions leading, cycles as
 * member) and a cycle sampling event for the calling thread's user-space
 * execution, then closes everything. It measures nothing and changes no host
 * setting; native.py turns the errno values into an explanation.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <linux/perf_event.h>
#include <linux/version.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
#define PF_PROBE_FORMAT_LOST PERF_FORMAT_LOST
#else
#define PF_PROBE_FORMAT_LOST 0u
#endif

static int open_event(unsigned long long config, int group, int sampling,
                      unsigned long long read_format)
{
    struct perf_event_attr attr;

    memset(&attr, 0, sizeof attr);
    attr.size = sizeof attr;
    attr.type = PERF_TYPE_HARDWARE;
    attr.config = config;
    attr.disabled = (group == -1) ? 1u : 0u;
    attr.exclude_kernel = 1;
    attr.exclude_hv = 1;
    attr.read_format = read_format;
    if (sampling)
    {
        attr.sample_period = 1000000;
        attr.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID;
    }
    return (int)syscall(SYS_perf_event_open, &attr, 0, -1, group,
                        PERF_FLAG_FD_CLOEXEC);
}

static const char *classify(int error)
{
    if ((error == EACCES) || (error == EPERM))
    {
        return "denied";
    }
    if ((error == ENOENT) || (error == ENODEV) || (error == EOPNOTSUPP))
    {
        return "unavailable";
    }
    return "error";
}

static void report(const char *name, int fd, int error, int last)
{
    if (fd >= 0)
    {
        (void)printf("\"%s\":{\"status\":\"opened\"}%s", name, last ? "" : ",");
    }
    else
    {
        (void)printf("\"%s\":{\"status\":\"%s\",\"errno\":%d}%s", name,
                     classify(error), error, last ? "" : ",");
    }
}

int main(void)
{
    unsigned long long group_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID |
                                      PERF_FORMAT_TOTAL_TIME_ENABLED |
                                      PERF_FORMAT_TOTAL_TIME_RUNNING;
    int leader = open_event(PERF_COUNT_HW_INSTRUCTIONS, -1, 0, group_format);
    int leader_error = errno;
    int member = -1;
    int member_error = 0;

    if (leader >= 0)
    {
        member = open_event(PERF_COUNT_HW_CPU_CYCLES, leader, 0, group_format);
        member_error = errno;
    }
    int sampler = open_event(PERF_COUNT_HW_CPU_CYCLES, -1, 1,
                             PERF_FORMAT_TOTAL_TIME_ENABLED |
                                 PERF_FORMAT_TOTAL_TIME_RUNNING);
    int sampler_error = errno;
    int lost = -1;
    int lost_error = ENOTSUP;

    if (PF_PROBE_FORMAT_LOST != 0u)
    {
        lost = open_event(PERF_COUNT_HW_CPU_CYCLES, -1, 1,
                          PERF_FORMAT_TOTAL_TIME_ENABLED |
                              PERF_FORMAT_TOTAL_TIME_RUNNING |
                              PF_PROBE_FORMAT_LOST);
        lost_error = errno;
    }

    (void)printf("{");
    if ((leader >= 0) && (member < 0))
    {
        /* The leader opened but the strict group failed as a unit. */
        report("counting_group", member, member_error, 0);
    }
    else
    {
        report("counting_group", leader, leader_error, 0);
    }
    report("sampling", sampler, sampler_error, 0);
    report("lost_count_format", lost, lost_error, 0);
    (void)printf("\"uapi_header_has_format_lost\":%s}\n",
                 (PF_PROBE_FORMAT_LOST != 0u) ? "true" : "false");

    int descriptors[] = {member, leader, sampler, lost};
    for (size_t i = 0; i < sizeof descriptors / sizeof descriptors[0]; ++i)
    {
        if (descriptors[i] >= 0)
        {
            (void)close(descriptors[i]);
        }
    }
    return 0;
}
