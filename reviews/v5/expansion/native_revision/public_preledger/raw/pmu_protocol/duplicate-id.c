#include "pmu.h"
#include <errno.h>
#include <string.h>

static PFPmuState fail(PFPmu *p, PFPmuState state, int error) {
    p->state = state;
    p->error = error;
    pf_pmu_close(p);
    return state;
}

void pf_pmu_close(PFPmu *p) {
    if (!p) return;
    while (p->opened) {
        int fd = p->descriptors[--p->opened];
        p->descriptors[p->opened] = -1;
        /* Linux close must not be retried after EINTR: the fd may be reused. */
        if (p->os.close(p->os.context, fd) < 0 && !p->cleanup_error)
            p->cleanup_error = errno;
    }
}

PFPmuState pf_pmu_open(PFPmu *p, PFPmuOS os,
                       const PFPmuEvent *events, size_t count) {
    if (!p) return PF_PMU_INVALID;
    *p = (PFPmu){0};
    p->state = PF_PMU_INVALID;
    p->failed_event = SIZE_MAX;
    for (size_t i = 0; i < PF_PMU_MAX; ++i) p->descriptors[i] = -1;
    if (!events || !count || count > PF_PMU_MAX || !os.open ||
        !os.control || !os.read || !os.close) return p->state;
    p->os = os;
    p->count = count;
    /* One PMU type per requested group. Cross-domain grouping is refused. */
    for (size_t i = 1; i < count; ++i)
        if (events[i].type != events[0].type) return p->state;
    for (size_t i = 0; i < count; ++i) {
        int fd = os.open(os.context, events[i], i ? p->descriptors[0] : -1);
        if (fd < 0) {
            int error = errno;
            p->failed_event = i;
            return fail(p, error == EACCES || error == EPERM ? PF_PMU_DENIED :
                        error == ENOENT || error == ENODEV || error == EOPNOTSUPP ||
                        error == EINVAL ? PF_PMU_UNAVAILABLE : PF_PMU_IO, error);
        }
        p->descriptors[p->opened++] = fd;
        if (os.control(os.context, fd, PF_PMU_ID, &p->ids[i]) < 0)
            return fail(p, PF_PMU_IO, errno);
        for (size_t j = 0; j < i; ++j)
            if (p->ids[j] == p->ids[i]) return fail(p, PF_PMU_MALFORMED, 0);
    }
    p->state = PF_PMU_READY;
    return p->state;
}

PFPmuState pf_pmu_start(PFPmu *p) {
    if (!p || p->state != PF_PMU_READY || p->opened != p->count)
        return PF_PMU_INVALID;
    if (p->os.control(p->os.context, p->descriptors[0], PF_PMU_RESET, NULL) < 0 ||
        p->os.control(p->os.context, p->descriptors[0], PF_PMU_ENABLE, NULL) < 0)
        return fail(p, PF_PMU_IO, errno);
    p->state = PF_PMU_ACTIVE;
    return p->state;
}

PFPmuState pf_pmu_stop(PFPmu *p) {
    uint64_t words[3 + 2 * PF_PMU_MAX];
    unsigned seen = 0;
    ptrdiff_t got = -1;
    if (!p || p->state != PF_PMU_ACTIVE || p->opened != p->count)
        return PF_PMU_INVALID;
    if (p->os.control(p->os.context, p->descriptors[0], PF_PMU_DISABLE, NULL) < 0)
        return fail(p, PF_PMU_IO, errno);
    size_t length = (3 + 2 * p->count) * sizeof(uint64_t);
    /* Bounded retry avoids a signal storm becoming an infinite test/run. */
    for (unsigned attempt = 0; attempt < 8; ++attempt) {
        got = p->os.read(p->os.context, p->descriptors[0], words, length);
        if (got >= 0 || errno != EINTR) break;
    }
    if (got < 0) return fail(p, PF_PMU_IO, errno);
    if (got == 0) return fail(p, PF_PMU_NEVER_RAN, 0);
    if ((size_t)got != length || words[0] != p->count)
        return fail(p, PF_PMU_MALFORMED, 0);
    p->enabled = words[1];
    p->running = words[2];
    if (p->running > p->enabled) return fail(p, PF_PMU_MALFORMED, 0);
    for (size_t i = 0; i < p->count; ++i) {
        size_t j;
        for (j = 0; j < p->count && p->ids[j] != words[4 + 2*i]; ++j) {}
        if (j == p->count)
            return fail(p, PF_PMU_MALFORMED, 0);
        seen |= 1u << j;
        p->raw[j] = words[3 + 2*i];
    }
    p->state = p->running ? PF_PMU_MEASURED : PF_PMU_NEVER_RAN;
    pf_pmu_close(p);
    return p->state;
}

static void copy_ring(void *out, const unsigned char *ring, size_t capacity,
                      uint64_t tail, size_t count) {
    unsigned char *bytes = out;
    for (size_t i = 0; i < count; ++i)
        bytes[i] = ring[(size_t)(tail + i) & (capacity - 1)];
}

int pf_ring_drain(const unsigned char *ring, size_t capacity, uint64_t head,
                  uint64_t *tail, size_t budget, PFRingStats *stats,
                  void (*sample)(void *, const PFSample *), void *context) {
    if (!ring || !tail || !stats || capacity < 8 || (capacity & (capacity-1)))
        return PF_RING_INVALID;
    if (head - *tail > capacity) return PF_RING_OVERFLOW;
    while (head != *tail && budget--) {
        uint32_t type;
        uint16_t size;
        unsigned char header[8];
        uint64_t available = head - *tail;
        if (available < 8) return PF_RING_PARTIAL;
        copy_ring(header, ring, capacity, *tail, sizeof header);
        memcpy(&type, header, sizeof type);
        memcpy(&size, header+6, sizeof size);
        if (size < 8 || size > capacity || size % 8) return PF_RING_INVALID;
        if (size > available) return PF_RING_PARTIAL;
        if (stats->records == UINT64_MAX) return PF_RING_OVERFLOW;
        if (type == 9) { /* PERF_RECORD_SAMPLE, declared fixed sample mask. */
            unsigned char payload[32];
            PFSample value;
            if (size != 40) return PF_RING_INVALID;
            if (stats->samples == UINT64_MAX) return PF_RING_OVERFLOW;
            copy_ring(payload, ring, capacity, *tail+8, sizeof payload);
            memcpy(&value.ip, payload, 8);
            memcpy(&value.pid, payload+8, 4);
            memcpy(&value.tid, payload+12, 4);
            memcpy(&value.time, payload+16, 8);
            memcpy(&value.period, payload+24, 8);
            if (sample) sample(context, &value);
            ++stats->samples;
        } else if (type == 2 || type == 13) { /* LOST / LOST_SAMPLES */
            uint64_t lost;
            if (size != (type == 2 ? 24 : 16)) return PF_RING_INVALID;
            copy_ring(&lost, ring, capacity, *tail+(type == 2 ? 16 : 8), 8);
            if (lost > UINT64_MAX-stats->lost) return PF_RING_OVERFLOW;
            stats->lost += lost;
        } else {
            if (stats->unknown == UINT64_MAX) return PF_RING_OVERFLOW;
            ++stats->unknown;
        }
        ++stats->records;
        *tail += size;
    }
    return PF_RING_OK;
}
