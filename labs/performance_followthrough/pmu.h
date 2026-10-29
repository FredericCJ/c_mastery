#ifndef CMASTERY_PMU_H
#define CMASTERY_PMU_H
#include <stddef.h>
#include <stdint.h>

#define PF_PMU_MAX 8

enum pf_pmu_state
{
    PF_PMU_EMPTY,
    PF_PMU_READY,
    PF_PMU_ACTIVE,
    PF_PMU_MEASURED,
    PF_PMU_DENIED,
    PF_PMU_UNAVAILABLE,
    PF_PMU_IO,
    PF_PMU_NEVER_RAN,
    PF_PMU_MALFORMED,
    PF_PMU_INVALID,
    PF_PMU_UNIMPLEMENTED
};

struct pf_pmu_event
{
    uint32_t type;
    uint64_t config;
};

enum pf_pmu_control
{
    PF_PMU_ID,
    PF_PMU_RESET,
    PF_PMU_ENABLE,
    PF_PMU_DISABLE
};

/* Supplied OS boundary: operations return -1 and set errno on failure.
 * read returns one GROUP|TIME_ENABLED|TIME_RUNNING|ID record. No fallback. */
struct pf_pmu_os
{
    void *context;
    int (*open)(void *, struct pf_pmu_event, int leader);
    int (*control)(void *, int, enum pf_pmu_control, uint64_t *);
    ptrdiff_t (*read)(void *, int, void *, size_t);
    int (*close)(void *, int);
};

struct pf_pmu
{
    struct pf_pmu_os os;
    enum pf_pmu_state state;
    size_t count, opened, failed_event;
    int descriptors[PF_PMU_MAX], error, cleanup_error;
    uint64_t ids[PF_PMU_MAX], raw[PF_PMU_MAX], enabled, running;
};
/* Fresh/quiescent, noncopied caller storage; same thread throughout. */
enum pf_pmu_state pf_pmu_open(struct pf_pmu *, struct pf_pmu_os,
                              const struct pf_pmu_event *, size_t);
enum pf_pmu_state pf_pmu_start(struct pf_pmu *);
enum pf_pmu_state pf_pmu_stop(struct pf_pmu *);
void pf_pmu_close(struct pf_pmu *);
const char *pf_pmu_state_name(enum pf_pmu_state);
struct pf_pmu_os pf_pmu_linux(void);

struct pf_sample
{
    uint64_t ip, time, period;
    uint32_t pid, tid;
};

struct pf_ring_stats
{
    uint64_t samples, lost, unknown, records;
};

enum pf_ring_result
{
    PF_RING_OK,
    PF_RING_PARTIAL,
    PF_RING_INVALID,
    PF_RING_OVERFLOW,
    PF_RING_UNIMPLEMENTED
};

/* Native-endian perf stream, SAMPLE=IP|TID|TIME|PERIOD, sample_id_all=0.
 * Successful records advance tail. Bad/partial record stays unconsumed.
 * callback observes borrowed copy synchronously; it cannot fail or retain it. */
int pf_ring_drain(const unsigned char *, size_t capacity, uint64_t head,
                  uint64_t *tail, size_t budget, struct pf_ring_stats *,
                  void (*sample)(void *, const struct pf_sample *), void *);
#endif
