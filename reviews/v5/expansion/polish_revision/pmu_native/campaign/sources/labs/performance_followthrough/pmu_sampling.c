/*
 * Supplied adapter: hardware-cycle sampling with final or concurrent
 * draining. Contracts: pmu_sampling.h, pmu_loss.h and pmu.h.
 */
#define _GNU_SOURCE
#include "pmu_sampling.h"

#include <errno.h>
#include <linux/perf_event.h>
#include <linux/version.h>
#include <poll.h>
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

/* PERF_FORMAT_LOST is an enumeration constant (not a macro) that the UAPI
 * headers define from Linux 6.0, so the header version is the build-time
 * test; the running kernel is tested separately by the open() result. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
#define PF_HAVE_FORMAT_LOST 1
#else
#define PF_HAVE_FORMAT_LOST 0
#endif

enum
{
    PF_READ_RETRIES = 8 /* bounded EINTR retries, as in the counting adapter */
};

int pf_ring_view_drain(struct pf_ring_view *view, size_t budget,
                       struct pf_ring_stats *stats, pf_sample_sink_fn sink,
                       void *context)
{
    /* Acquire: every byte the kernel published before data_head is visible. */
    uint64_t head = __atomic_load_n(view->head, __ATOMIC_ACQUIRE);
    /* Only this consumer writes data_tail, so its own last value suffices. */
    uint64_t tail = __atomic_load_n(view->tail, __ATOMIC_RELAXED);
    int status = pf_ring_drain(view->data, view->capacity, head, &tail, budget,
                               stats, sink, context);

    /* Release: all reads of consumed records happen before the kernel may
     * reuse their bytes. On failure, tail stops at the last complete record. */
    __atomic_store_n(view->tail, tail, __ATOMIC_RELEASE);
    return status;
}

static void retain_sample(void *context, const struct pf_sample *sample)
{
    struct pf_sample_buffer *buffer = context;

    if (buffer->count < buffer->capacity)
    {
        buffer->items[buffer->count] = *sample;
        buffer->count++;
    }
    else
    {
        buffer->not_retained++;
    }
}

static int open_sampler(const struct pf_sampling_config *config, bool with_lost)
{
    struct perf_event_attr attr;

    memset(&attr, 0, sizeof attr);
    attr.size = sizeof attr;
    attr.type = PERF_TYPE_HARDWARE;
    attr.config = PERF_COUNT_HW_CPU_CYCLES;
    attr.disabled = 1;
    attr.exclude_kernel = 1;
    attr.exclude_hv = 1;
    attr.sample_period = config->sample_period_cycles;
    attr.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_TIME |
                       PERF_SAMPLE_PERIOD;
    attr.wakeup_events = 1;
    attr.precise_ip = config->precise_ip;
    attr.read_format =
        PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
#if PF_HAVE_FORMAT_LOST
    if (with_lost)
    {
        attr.read_format |= PERF_FORMAT_LOST;
    }
#else
    (void)with_lost;
#endif
    /* pid 0, cpu -1: the calling thread on any CPU, no inheritance. */
    return (int)syscall(SYS_perf_event_open, &attr, 0, -1, -1,
                        PERF_FLAG_FD_CLOEXEC);
}

/* Open the sampler, asking for the kernel loss count when possible. A kernel
 * that rejects only that read format (EINVAL) falls back to opening without
 * it; any other rejection (for example an unsupported precise_ip) stays a
 * failure, reported with the errno of the request without the LOST format.
 * Support is recorded only once an open has shown it. */
static int open_with_loss_count(const struct pf_sampling_config *config,
                                struct pf_sampling_result *result)
{
    if (!config->request_lost_count)
    {
        result->lost_support = PF_LOST_NOT_REQUESTED;
        return open_sampler(config, false);
    }
    if (!PF_HAVE_FORMAT_LOST)
    {
        result->lost_support = PF_LOST_HEADER_MISSING;
        return open_sampler(config, false);
    }
    result->lost_support = PF_LOST_UNDETERMINED;
    int fd = open_sampler(config, true);
    if (fd >= 0)
    {
        result->lost_support = PF_LOST_SUPPORTED;
        return fd;
    }
    if (errno != EINVAL)
    {
        return fd;
    }
    fd = open_sampler(config, false);
    if (fd >= 0)
    {
        result->lost_support = PF_LOST_KERNEL_REJECTED;
    }
    return fd;
}

struct drainer
{
    struct pf_ring_view view;
    int fd;
    unsigned interval_ms;
    atomic_bool stop;
    struct pf_ring_stats *stats;
    struct pf_sample_buffer *samples;
    int status;
    uint64_t drains;
};

static void *drainer_main(void *argument)
{
    struct drainer *drainer = argument;

    while (!atomic_load_explicit(&drainer->stop, memory_order_acquire))
    {
        struct pollfd waiter = {.fd = drainer->fd, .events = POLLIN};

        /* Timeout, EINTR and readiness all lead to the same bounded drain. */
        (void)poll(&waiter, 1, (int)drainer->interval_ms);
        drainer->status =
            pf_ring_view_drain(&drainer->view, SIZE_MAX, drainer->stats,
                               retain_sample, drainer->samples);
        drainer->drains++;
        if (drainer->status != PF_RING_OK)
        {
            break; /* the final drain reports the malformed record */
        }
    }
    return NULL;
}

static enum pf_reading_status read_counts(int fd, bool with_lost,
                                          struct pf_sample_reading *reading)
{
    uint64_t words[4] = {0};
    ssize_t got = -1;
    size_t length = (with_lost ? 4u : 3u) * sizeof words[0];

    for (unsigned attempt = 0; attempt < PF_READ_RETRIES; ++attempt)
    {
        got = read(fd, words, length);
        if ((got >= 0) || (errno != EINTR))
        {
            break;
        }
    }
    if (got < 0)
    {
        return PF_READING_SHORT;
    }
    return pf_sample_reading_decode(words, (size_t)got, with_lost, reading);
}

static bool valid_layout(const struct perf_event_mmap_page *meta,
                         size_t mapping_length, size_t page_size)
{
    uint64_t offset = meta->data_offset;
    uint64_t size = meta->data_size;

    /* Kernels before 4.1 leave data_offset zero: the data starts one page in. */
    if (offset == 0u)
    {
        offset = page_size;
        size = mapping_length - page_size;
    }
    return (offset <= mapping_length) && (size != 0u) &&
           (size <= mapping_length - offset) && ((size & (size - 1u)) == 0u);
}

static const char *classify_open_error(int error)
{
    if ((error == EACCES) || (error == EPERM))
    {
        return "denied";
    }
    return "unavailable";
}

/* Validate the kernel's description of the mapped ring and describe its
 * data area. Returns false when the layout cannot be trusted. */
static bool ring_view(void *mapping, size_t length, size_t page,
                      struct pf_ring_view *view)
{
    struct perf_event_mmap_page *meta = mapping;

    if (!valid_layout(meta, length, page))
    {
        return false;
    }
    uint64_t data_offset = meta->data_offset ? meta->data_offset : page;
    uint64_t data_size = meta->data_size ? meta->data_size : length - page;
    *view = (struct pf_ring_view){
        .data = (const unsigned char *)mapping + data_offset,
        .capacity = (size_t)data_size,
        .head = (uint64_t *)&meta->data_head,
        .tail = (uint64_t *)&meta->data_tail,
    };
    return true;
}

/* Run the workload between reset/enable and disable; in concurrent mode a
 * drainer thread consumes the ring meanwhile and is joined before the final
 * drain on this thread. Returns false, with the workload not run, when the
 * drainer thread cannot be started; otherwise stores the ring status. */
static bool observe(int fd, const struct pf_ring_view *view,
                    const struct pf_sampling_config *config,
                    pf_workload_fn workload, void *context,
                    struct pf_sampling_result *result, int *ring_status)
{
    struct drainer drainer = {
        .view = *view,
        .fd = fd,
        .interval_ms = config->poll_interval_ms ? config->poll_interval_ms : 1u,
        .stats = &result->ring,
        .samples = &result->samples,
        .status = PF_RING_OK,
    };
    pthread_t thread;
    bool concurrent = (config->drain == PF_DRAIN_CONCURRENT);

    atomic_init(&drainer.stop, false);
    if (concurrent &&
        (pthread_create(&thread, NULL, drainer_main, &drainer) != 0))
    {
        return false;
    }
    if ((ioctl(fd, PERF_EVENT_IOC_RESET, 0) != 0) ||
        (ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) != 0))
    {
        result->open_errno = errno;
    }
    else
    {
        workload(context);
    }
    if (ioctl(fd, PERF_EVENT_IOC_DISABLE, 0) != 0)
    {
        result->open_errno = errno;
    }
    if (concurrent)
    {
        atomic_store_explicit(&drainer.stop, true, memory_order_release);
        (void)pthread_join(thread, NULL); /* the drainer releases data_tail */
    }
    /* Final drain on this thread: nothing else consumes the ring now. */
    *ring_status = drainer.status;
    if (*ring_status == PF_RING_OK)
    {
        *ring_status =
            pf_ring_view_drain(&drainer.view, SIZE_MAX, &result->ring,
                               retain_sample, &result->samples);
        drainer.drains++;
    }
    result->drains = drainer.drains;
    return true;
}

/* Read the disabled event's counts, reconcile loss and classify. */
static void classify(int fd, int ring_status, struct pf_sampling_result *result)
{
    bool with_lost = (result->lost_support == PF_LOST_SUPPORTED);
    enum pf_reading_status reading =
        read_counts(fd, with_lost, &result->reading);
    struct pf_loss_inputs loss = {
        .inband_lost = result->ring.lost,
        .hardware_lost = result->ring.hardware_lost,
        .kernel_count_known = with_lost && (reading == PF_READING_OK),
        .kernel_lost = result->reading.lost,
    };

    pf_loss_reconcile(&loss, &result->loss);
    if ((ring_status != PF_RING_OK) || (reading != PF_READING_OK) ||
        (result->open_errno != 0))
    {
        result->status = "invalid";
    }
    else if (result->reading.running_ns == 0u)
    {
        result->status = "never-scheduled";
    }
    else if (result->ring.samples == 0u)
    {
        result->status = "no-samples";
    }
    else
    {
        result->status = "measured";
    }
}

void pf_sampling_run(const struct pf_sampling_config *config,
                     pf_workload_fn workload, void *context,
                     struct pf_sampling_result *result)
{
    struct pf_sample_buffer samples = {
        .items = result->samples.items,
        .capacity = config->sample_capacity,
    };

    *result = (struct pf_sampling_result){.status = "invalid"};
    result->samples = samples;
    /* Until a reading proves otherwise nothing is known about loss: an
     * early failure must report unknown, not a zero-initialized complete
     * account. */
    const struct pf_loss_inputs nothing_observed = {.kernel_count_known =
                                                        false};
    pf_loss_reconcile(&nothing_observed, &result->loss);

    long page = sysconf(_SC_PAGESIZE);
    if ((page <= 0) || (((size_t)page & ((size_t)page - 1u)) != 0u) ||
        (config->data_pages == 0u) ||
        ((config->data_pages & (config->data_pages - 1u)) != 0u))
    {
        return;
    }

    int fd = open_with_loss_count(config, result);
    if (fd < 0)
    {
        result->open_errno = errno;
        result->status = classify_open_error(errno);
        return;
    }

    size_t length = (size_t)page * (1u + config->data_pages);
    void *mapping =
        mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED)
    {
        result->open_errno = errno;
        result->status = "map-failed";
        if (close(fd) != 0)
        {
            result->cleanup_errno = errno;
        }
        return;
    }

    struct pf_ring_view view;
    int ring_status = PF_RING_INVALID;
    if (ring_view(mapping, length, (size_t)page, &view) &&
        observe(fd, &view, config, workload, context, result, &ring_status))
    {
        classify(fd, ring_status, result);
    }
    result->parser_status = ring_status;
    if (munmap(mapping, length) != 0)
    {
        result->cleanup_errno = errno;
    }
    if ((close(fd) != 0) && (result->cleanup_errno == 0))
    {
        result->cleanup_errno = errno;
    }
}

const char *pf_lost_support_name(enum pf_lost_support support)
{
    switch (support)
    {
        case PF_LOST_SUPPORTED:
            return "supported";
        case PF_LOST_NOT_REQUESTED:
            return "not-requested";
        case PF_LOST_HEADER_MISSING:
            return "header-missing";
        case PF_LOST_KERNEL_REJECTED:
            return "kernel-rejected";
        case PF_LOST_UNDETERMINED:
            return "undetermined";
        default:
            return "invalid";
    }
}

const char *pf_drain_mode_name(enum pf_drain_mode mode)
{
    switch (mode)
    {
        case PF_DRAIN_FINAL:
            return "final";
        case PF_DRAIN_CONCURRENT:
            return "concurrent";
        default:
            return "invalid";
    }
}
