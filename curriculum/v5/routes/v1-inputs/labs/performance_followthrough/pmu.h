#ifndef CMASTERY_PMU_H
#define CMASTERY_PMU_H
#include <stddef.h>
#include <stdint.h>

#define PF_PMU_MAX 8
typedef enum {
    PF_PMU_EMPTY, PF_PMU_READY, PF_PMU_ACTIVE, PF_PMU_MEASURED,
    PF_PMU_DENIED, PF_PMU_UNAVAILABLE, PF_PMU_IO, PF_PMU_NEVER_RAN,
    PF_PMU_MALFORMED, PF_PMU_INVALID, PF_PMU_UNIMPLEMENTED
} PFPmuState;
typedef struct { uint32_t type; uint64_t config; } PFPmuEvent;
enum PFPmuControl { PF_PMU_ID, PF_PMU_RESET, PF_PMU_ENABLE, PF_PMU_DISABLE };
/* Supplied OS boundary: operations return -1 and set errno on failure.
 * read returns one GROUP|TIME_ENABLED|TIME_RUNNING|ID record. No fallback. */
typedef struct {
    void *context;
    int (*open)(void *, PFPmuEvent, int leader);
    int (*control)(void *, int, enum PFPmuControl, uint64_t *);
    ptrdiff_t (*read)(void *, int, void *, size_t);
    int (*close)(void *, int);
} PFPmuOS;
typedef struct {
    PFPmuOS os;
    PFPmuState state;
    size_t count, opened, failed_event;
    int descriptors[PF_PMU_MAX], error, cleanup_error;
    uint64_t ids[PF_PMU_MAX], raw[PF_PMU_MAX], enabled, running;
} PFPmu;
/* Fresh/quiescent, noncopied caller storage; same thread throughout. */
PFPmuState pf_pmu_open(PFPmu *, PFPmuOS, const PFPmuEvent *, size_t);
PFPmuState pf_pmu_start(PFPmu *);
PFPmuState pf_pmu_stop(PFPmu *);
void pf_pmu_close(PFPmu *);
const char *pf_pmu_state_name(PFPmuState);
PFPmuOS pf_pmu_linux(void);

typedef struct { uint64_t ip, time, period; uint32_t pid, tid; } PFSample;
typedef struct {
    uint64_t samples, lost, unknown, records;
} PFRingStats;
enum PFRingResult { PF_RING_OK, PF_RING_PARTIAL, PF_RING_INVALID,
                    PF_RING_OVERFLOW, PF_RING_UNIMPLEMENTED };
/* Native-endian perf stream, SAMPLE=IP|TID|TIME|PERIOD, sample_id_all=0.
 * Successful records advance tail. Bad/partial record stays unconsumed.
 * callback observes borrowed copy synchronously; it cannot fail or retain it. */
int pf_ring_drain(const unsigned char *, size_t capacity, uint64_t head,
                  uint64_t *tail, size_t budget, PFRingStats *,
                  void (*sample)(void *, const PFSample *), void *);
#endif
