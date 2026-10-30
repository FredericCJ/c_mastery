#define _GNU_SOURCE
#include "pmu.h"
#include "publication.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "PF_PROTOCOL_ASSERT:%d:%s\n", __LINE__, #x); \
            exit(1);                                                     \
        }                                                                \
    } while (0)

struct pf_fake
{
    int open_fail, control_fail, close_fail, opens, closes, reads, interrupts;
    int open_error, controls, leaders[8];
    ptrdiff_t length;
    uint64_t words[7];
};

static struct pf_fake fixture(void)
{
    return (struct pf_fake){.open_fail = -1,
                            .control_fail = -1,
                            .close_fail = -1,
                            .open_error = EPERM,
                            .length = 56,
                            .words = {2, 200, 100, 900, 101, 0, 100}};
}

static int mock_open(void *context, struct pf_pmu_event event, int leader)
{
    struct pf_fake *f = context;
    CHECK(event.type == 0);
    int i = f->opens++;
    f->leaders[i] = leader;
    if (i == f->open_fail)
    {
        errno = f->open_error;
        return -1;
    }
    return 20 + i;
}

static int mock_control(void *context, int fd, enum pf_pmu_control op,
                        uint64_t *id)
{
    struct pf_fake *f = context;
    CHECK(fd >= 20 && fd < 22);
    if (f->controls++ == f->control_fail)
    {
        errno = EIO;
        return -1;
    }
    if (op == PF_PMU_ID)
    {
        *id = (uint64_t)(fd + 80);
    }
    else
    {
        CHECK(fd == 20);
    }
    return 0;
}

static ptrdiff_t mock_read(void *context, int fd, void *out, size_t n)
{
    struct pf_fake *f = context;
    CHECK(fd == 20 && n == 56);
    ++f->reads;
    if (f->interrupts-- > 0)
    {
        errno = EINTR;
        return -1;
    }
    if (f->length < 0)
    {
        errno = EIO;
        return -1;
    }
    if (f->length > 0)
    {
        memcpy(out, f->words, (size_t)f->length);
    }
    return f->length;
}

static int mock_close(void *context, int fd)
{
    struct pf_fake *f = context;
    CHECK(fd == 20 + f->opens - 1 - f->closes - (f->open_fail >= 0 ? 1 : 0));
    ++f->closes;
    if (fd == f->close_fail)
    {
        errno = EINTR;
        return -1;
    }
    return 0;
}

static struct pf_pmu_os mock_os(struct pf_fake *f)
{
    return (struct pf_pmu_os){f, mock_open, mock_control, mock_read,
                              mock_close};
}

static enum pf_pmu_state collect(struct pf_fake *f, struct pf_pmu *p)
{
    const struct pf_pmu_event events[] = {{0, 1}, {0, 0}};
    enum pf_pmu_state state = pf_pmu_open(p, mock_os(f), events, 2);
    if (state == PF_PMU_READY)
    {
        state = pf_pmu_start(p);
    }
    if (state == PF_PMU_ACTIVE)
    {
        state = pf_pmu_stop(p);
    }
    return state;
}

static void acquisition(void)
{
    struct pf_fake f = fixture();
    struct pf_pmu p;
    CHECK(collect(&f, &p) == PF_PMU_MEASURED);
    CHECK(p.raw[0] == 0 && p.raw[1] == 900 && p.running == 100 &&
          p.enabled == 200);
    CHECK(f.leaders[0] == -1 && f.leaders[1] == 20 && f.closes == 2 &&
          !p.opened);
    pf_pmu_close(&p);
    CHECK(f.closes == 2);
    CHECK(pf_pmu_start(&p) == PF_PMU_INVALID);
    for (int failure = 0; failure < 2; ++failure)
    {
        f = fixture();
        f.open_fail = failure;
        CHECK(collect(&f, &p) == PF_PMU_DENIED &&
              p.failed_event == (size_t)failure);
        CHECK(f.opens == failure + 1 && f.closes == failure && f.reads == 0 &&
              !p.opened);
    }
    f = fixture();
    f.open_fail = 1;
    f.open_error = ENODEV;
    CHECK(collect(&f, &p) == PF_PMU_UNAVAILABLE && f.closes == 1);
    for (int control = 0; control < 5; ++control)
    {
        f = fixture();
        f.control_fail = control;
        CHECK(collect(&f, &p) == PF_PMU_IO && !p.opened && p.error == EIO);
        CHECK(f.closes == f.opens);
    }
    f = fixture();
    f.words[2] = 0;
    CHECK(collect(&f, &p) == PF_PMU_NEVER_RAN && p.raw[1] == 900 &&
          f.closes == 2);
    f = fixture();
    f.length = 0;
    CHECK(collect(&f, &p) == PF_PMU_NEVER_RAN);
    for (ptrdiff_t n = 1; n < 56; ++n)
    {
        f = fixture();
        f.length = n;
        CHECK(collect(&f, &p) == PF_PMU_MALFORMED && f.closes == 2);
    }
    for (unsigned field = 0; field < 4; ++field)
    {
        f = fixture();
        if (field == 0)
        {
            f.words[0] = 1;
        }
        if (field == 1)
        {
            f.words[2] = 201;
        }
        if (field == 2)
        {
            f.words[4] = 100;
        }
        if (field == 3)
        {
            f.words[4] = 999;
        }
        CHECK(collect(&f, &p) == PF_PMU_MALFORMED && f.closes == 2);
    }
    f = fixture();
    f.interrupts = 2;
    CHECK(collect(&f, &p) == PF_PMU_MEASURED && f.reads == 3);
    f = fixture();
    f.interrupts = 9;
    CHECK(collect(&f, &p) == PF_PMU_IO && f.reads == 8 && f.closes == 2 &&
          p.error == EINTR);
    f = fixture();
    f.length = -1;
    CHECK(collect(&f, &p) == PF_PMU_IO && f.closes == 2);
    f = fixture();
    f.close_fail = 21;
    CHECK(collect(&f, &p) == PF_PMU_MEASURED && p.cleanup_error == EINTR &&
          f.closes == 2);
    const struct pf_pmu_event mixed[] = {{0, 1}, {4, 0}};
    f = fixture();
    CHECK(pf_pmu_open(&p, mock_os(&f), mixed, 2) == PF_PMU_INVALID &&
          f.opens == 0);
    puts("PF_PROTOCOL:strict-group:leader-member-control-read-time-id-cleanup");
}

static void write_ring(unsigned char *ring, size_t n, uint64_t pos,
                       const void *p, size_t length)
{
    const unsigned char *bytes = p;
    for (size_t i = 0; i < length; ++i)
    {
        ring[(size_t)(pos + i) & (n - 1)] = bytes[i];
    }
}

static unsigned callbacks;

static void sample(void *unused, const struct pf_sample *s)
{
    (void)unused;
    CHECK(s->ip == 0x1234 && s->pid == 7 && s->tid == 9 && s->time == 123 &&
          s->period == 4000);
    ++callbacks;
}

static void ring(void)
{
    unsigned char bytes[64] = {0};

    struct
    {
        uint32_t type;
        uint16_t misc, size;
        uint64_t ip;
        uint32_t pid, tid;
        uint64_t time, period;
    } record = {9, 0, 40, 0x1234, 7, 9, 123, 4000};

    CHECK(sizeof record == 40);
    for (unsigned offset = 0; offset < 64; ++offset)
    {
        uint64_t tail = offset;
        struct pf_ring_stats stats = {0};
        callbacks = 0;
        write_ring(bytes, 64, tail, &record, 40);
        CHECK(pf_ring_drain(bytes, 64, tail + 40, &tail, 1, &stats, sample,
                            NULL) == PF_RING_OK);
        CHECK(stats.samples == 1 && stats.records == 1 && tail == offset + 40 &&
              callbacks == 1);
    }
    for (unsigned length = 1; length < 40; ++length)
    {
        uint64_t tail = 61;
        struct pf_ring_stats stats = {0};
        write_ring(bytes, 64, tail, &record, 40);
        CHECK(pf_ring_drain(bytes, 64, tail + length, &tail, 2, &stats, NULL,
                            NULL) == PF_RING_PARTIAL &&
              tail == 61 && stats.records == 0);
    }
    for (unsigned size = 0; size < 72; ++size)
    {
        if (size == 40)
        {
            continue;
        }
        uint64_t tail = 0;
        struct pf_ring_stats stats = {0};
        record.size = (uint16_t)size;
        write_ring(bytes, 64, 0, &record, 40);
        CHECK(pf_ring_drain(bytes, 64, 64, &tail, 1, &stats, NULL, NULL) ==
                  PF_RING_INVALID &&
              tail == 0);
    }
    uint64_t tail = 0;
    struct pf_ring_stats stats = {0};
    CHECK(pf_ring_drain(bytes, 64, 65, &tail, 1, &stats, NULL, NULL) ==
          PF_RING_OVERFLOW);

    struct
    {
        uint32_t type;
        uint16_t misc, size;
        uint64_t id, lost;
    } lost = {2, 0, 24, 99, 12};

    write_ring(bytes, 64, 0, &lost, sizeof lost);
    CHECK(pf_ring_drain(bytes, 64, 24, &tail, 1, &stats, NULL, NULL) ==
              PF_RING_OK &&
          stats.lost == 12 && tail == 24);
    tail = 0;
    stats.lost = UINT64_MAX;
    CHECK(pf_ring_drain(bytes, 64, 24, &tail, 1, &stats, NULL, NULL) ==
              PF_RING_OVERFLOW &&
          tail == 0);

    /* PERF_RECORD_LOST_SAMPLES (hardware drops) is counted separately and
     * never added to the in-band ring loss. */
    struct
    {
        uint32_t type;
        uint16_t misc, size;
        uint64_t lost;
    } hardware = {13, 0, 16, 5};

    write_ring(bytes, 64, 0, &hardware, sizeof hardware);
    tail = 0;
    stats = (struct pf_ring_stats){0};
    CHECK(pf_ring_drain(bytes, 64, 16, &tail, 1, &stats, NULL, NULL) ==
              PF_RING_OK &&
          stats.hardware_lost == 5 && stats.lost == 0 && tail == 16);
    tail = 0;
    stats = (struct pf_ring_stats){.hardware_lost = UINT64_MAX};
    CHECK(pf_ring_drain(bytes, 64, 16, &tail, 1, &stats, NULL, NULL) ==
              PF_RING_OVERFLOW &&
          tail == 0);
    tail = 0;
    lost.type = 99;
    lost.size = 8;
    write_ring(bytes, 64, 0, &lost, 8);
    stats = (struct pf_ring_stats){0};
    CHECK(pf_ring_drain(bytes, 64, 8, &tail, 0, &stats, NULL, NULL) ==
              PF_RING_OK &&
          tail == 0);
    CHECK(pf_ring_drain(bytes, 64, 8, &tail, 1, &stats, NULL, NULL) ==
              PF_RING_OK &&
          stats.unknown == 1);
    tail = UINT64_MAX - 7;
    stats = (struct pf_ring_stats){0};
    write_ring(bytes, 64, tail, &lost, 8);
    CHECK(pf_ring_drain(bytes, 64, 0, &tail, 1, &stats, NULL, NULL) ==
              PF_RING_OK &&
          tail == 0);
    puts(
        "PF_PROTOCOL:ring:all-wrap-origins-partial-malformed-lost-budget-overflow");
}

struct pf_sync
{
    unsigned calls, fail;
};

static int inject_sync(void *context, int fd)
{
    struct pf_sync *s = context;
    if (++s->calls == s->fail)
    {
        errno = EIO;
        return -1;
    }
    return fsync(fd);
}

static void publication(void)
{
    char dir[] = "/tmp/cmastery-publish-XXXXXX";
    CHECK(mkdtemp(dir));
    char stage[160], target[160];
    CHECK(snprintf(stage, sizeof stage, "%s/stage", dir) > 0);
    CHECK(snprintf(target, sizeof target, "%s/target", dir) > 0);
    int dfd = open(dir, O_RDONLY | O_DIRECTORY);
    CHECK(dfd >= 0);
    for (unsigned fail = 0; fail < 3; ++fail)
    {
        int prior = open(target, O_CREAT | O_TRUNC | O_RDWR, 0600);
        CHECK(prior >= 0 && write(prior, "old", 3) == 3);
        int fd = open(stage, O_CREAT | O_EXCL | O_RDWR, 0600);
        CHECK(fd >= 0 && write(fd, "new", 3) == 3);
        struct pf_sync sync = {0, fail};
        struct pf_publication p =
            pf_publish(fd, dfd, stage, target,
                       (struct pf_synchronizer){&sync, inject_sync});
        CHECK(p.calls == sync.calls && p.calls == (fail == 1 ? 1u : 2u));
        CHECK(p.state == (fail == 1   ? PF_STAGE_RETAINED
                          : fail == 2 ? PF_VISIBLE_UNCONFIRMED
                                      : PF_SYNCED));
        CHECK(p.error == (fail ? EIO : 0));
        int actual = open(target, O_RDONLY);
        char data[3];
        CHECK(actual >= 0 && read(actual, data, 3) == 3);
        CHECK(!memcmp(data, fail == 1 ? "old" : "new", 3));
        CHECK(!close(actual));
        CHECK((access(stage, F_OK) == 0) == (fail == 1));
        CHECK(!close(prior) && !close(fd) && !unlink(target));
        if (fail == 1)
        {
            CHECK(!unlink(stage));
        }
    }
    CHECK(!close(dfd) && !rmdir(dir));
    puts(
        "PF_PROTOCOL:publication:actual-sync-seam-stage-retained-visible-unconfirmed-cleanup");
}

int main(void)
{
    acquisition();
    ring();
    publication();
    puts("PF_PROTOCOL_PASS");
    return 0;
}
