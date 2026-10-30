#define _GNU_SOURCE
#include "context.h"
#include "test.h"
#include <errno.h>
#include <inttypes.h>
#include <sched.h>
#include <signal.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

enum
{
    NS_PER_SECOND = 1000000000,
    SAMPLE_COUNT = 12,
    SAMPLE_PERIOD_NS = 2000000,
    BUSY_LOOP_ITERATIONS = 10000,
    /* 0 + 1 + ... + 9999 */
    BUSY_LOOP_SUM = 49995000,
    LOWERED_NOFILE_SOFT_LIMIT = 32,
    CHILD_LIMIT_FAILED_STATUS = 98,
    TIMER_DELAY_US = 1000,
    INTERRUPTED_SLEEP_NS = 10000000
};

/* Converts a monotonic timespec to nanoseconds, rejecting values that would
 * not fit in uint64_t or are not normalized. */
static uint64_t ns(struct timespec t)
{
    CHECK(t.tv_sec >= 0 && (uint64_t)t.tv_sec <= UINT64_MAX / NS_PER_SECOND &&
          t.tv_nsec >= 0 && t.tv_nsec < NS_PER_SECOND);
    return (uint64_t)t.tv_sec * NS_PER_SECOND + (uint64_t)t.tv_nsec;
}

static struct timespec ts(uint64_t nanoseconds)
{
    struct timespec t = {(time_t)(nanoseconds / NS_PER_SECOND),
                         (long)(nanoseconds % NS_PER_SECOND)};
    CHECK((uint64_t)t.tv_sec == nanoseconds / NS_PER_SECOND);
    return t;
}

static volatile sig_atomic_t signals;

static void caught(int sig)
{
    (void)sig;
    ++signals;
}

/* Pure arithmetic over injected tick values: no host clock is read and no
 * suspend or wall-clock jump is performed. */
static void models(void)
{
    uint64_t deadline = 77;
    CHECK(pc_deadline(100, 20, &deadline) == 0 && deadline == 120);
    CHECK(pc_remaining(deadline, 105) == 15 &&
          pc_remaining(deadline, 120) == 0 && pc_remaining(deadline, 121) == 0);
    CHECK(pc_deadline(UINT64_MAX, 1, &deadline) == ERANGE && deadline == 120);
    CHECK(pc_deadline(UINT64_MAX, 0, &deadline) == 0 && deadline == UINT64_MAX);
    CHECK(pc_deadline(0, 0, NULL) == EINVAL);
    /* Same 15 active ticks + 100 suspended ticks; no host clock or suspend changed. */
    CHECK(pc_remaining(120, 115) == 5 && pc_remaining(120, 215) == 0);
    /* A wall clock that jumps (forward, backward) is irrelevant: remaining
     * time depends only on the monotonic ticks passed to pc_remaining. */
    int64_t wall[] = {1000, 1005, 500, 9000};
    for (size_t i = 0; i < 4; ++i)
    {
        (void)wall[i];
        CHECK(pc_remaining(120, 100 + i * 5) == 20 - i * 5);
    }
    uint64_t planned[] = {5, 10, 20};
    uint64_t dispatch[] = {7, 13, 19};
    uint64_t late[3];
    unsigned expired = 0;
    for (size_t i = 0; i < 3; ++i)
    {
        late[i] = dispatch[i] > planned[i] ? dispatch[i] - planned[i] : 0;
        expired += planned[i] <= 12;
    }
    CHECK(late[0] == 2 && late[1] == 3 && late[2] == 0 && expired == 2);
    /* Changed interference model: an unbounded competing interval destroys any empirical upper bound. */
    CHECK(pc_remaining(20, 120) == 0);
    puts(
        "PC_CLOCK_MODEL wall_jump_independent active_remaining=5 suspend_remaining=0 lateness=2,3,0 expired=2 interference_lateness=100");
}

/* Forked child: lowers its own soft RLIMIT_NOFILE and reports through `fd`
 * whether the new value was observed. Never returns. */
static void lower_limit_in_child(int fd, struct rlimit before)
{
    struct rlimit smaller = before;
    if (smaller.rlim_cur > LOWERED_NOFILE_SOFT_LIMIT)
    {
        smaller.rlim_cur = LOWERED_NOFILE_SOFT_LIMIT;
    }
    int ok = setrlimit(RLIMIT_NOFILE, &smaller) == 0;
    struct rlimit seen;
    if (getrlimit(RLIMIT_NOFILE, &seen) || seen.rlim_cur != smaller.rlim_cur)
    {
        ok = 0;
    }
    ssize_t sent;
    do
    {
        sent = write(fd, &ok, sizeof ok);
    } while (sent < 0 && errno == EINTR);
    _exit(ok && sent == (ssize_t)sizeof ok ? 0 : CHILD_LIMIT_FAILED_STATUS);
}

/* Resource limits are per process: a child lowering its limit leaves the
 * parent's unchanged. Scheduling and affinity are only read, never changed. */
static void limits(void)
{
    struct rlimit before;
    struct rlimit after;
    CHECK(getrlimit(RLIMIT_NOFILE, &before) == 0);
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CHECK(sched_getaffinity(0, sizeof mask, &mask) == 0);
    int count = CPU_COUNT(&mask);
    CHECK(count > 0);
    int policy = sched_getscheduler(0);
    struct sched_param param;
    CHECK(policy >= 0 && sched_getparam(0, &param) == 0);
    printf("PC_SETTINGS soft=%llu hard=%llu cpus=%d policy=%d priority=%d\n",
           (unsigned long long)before.rlim_cur,
           (unsigned long long)before.rlim_max, count, policy,
           param.sched_priority);
    int channel[2];
    int status;
    CHECK(pipe(channel) == 0);
    pid_t pid = fork();
    CHECK(pid >= 0);
    if (!pid)
    {
        close(channel[0]);
        lower_limit_in_child(channel[1], before);
    }
    close(channel[1]);
    int ok = 0;
    CHECK(read(channel[0], &ok, sizeof ok) == sizeof ok && ok);
    close(channel[0]);
    CHECK(waitpid(pid, &status, 0) == pid && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    CHECK(getrlimit(RLIMIT_NOFILE, &after) == 0 &&
          before.rlim_cur == after.rlim_cur &&
          before.rlim_max == after.rlim_max);
    /* Permission and hotplug decisions are injected; no scheduler/host affinity changes. */
    unsigned accepted = 4;
    CHECK(pc_request(7, 3, EPERM, &accepted) == EPERM && accepted == 4);
    CHECK(pc_request(7, 3, EINVAL, &accepted) == EINVAL && accepted == 4);
    CHECK(pc_request(7, 3, 0, &accepted) == 0 && accepted == 3);
    unsigned inspected_cpu = 3;
    unsigned available_now = 1;
    CHECK(pc_request(available_now, inspected_cpu, 0, &accepted) == EINVAL &&
          accepted == 3);
    puts(
        "PC_LIMIT child_lowered parent_unchanged errors=EPERM,EINVAL cpu_snapshot_stale=model");
}

/* Sleeps to an absolute CLOCK_MONOTONIC deadline, resuming after signals.
 * Returns how many times the sleep was interrupted. */
static int sleep_until(uint64_t deadline_ns)
{
    struct timespec target = ts(deadline_ns);
    int interrupted = 0;
    int error;
    do
    {
        error = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &target, NULL);
        if (error == EINTR)
        {
            ++interrupted;
        }
    } while (error == EINTR);
    CHECK(error == 0);
    return interrupted;
}

/* Twelve absolute-deadline wakeups; each row records planned, woken,
 * dispatched and finished times so the campaign can bound ordering. The
 * lateness is an observation on a shared host, not a bound. */
static void monotonic_samples(void)
{
    struct timespec start;
    struct timespec cpu_start;
    struct timespec cpu_end;
    struct timespec resolution;
    CHECK(clock_getres(CLOCK_MONOTONIC, &resolution) == 0 &&
          clock_gettime(CLOCK_MONOTONIC, &start) == 0 &&
          clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_start) == 0);
    uint64_t origin = ns(start);
    printf("PC_RESOLUTION ns=%" PRIu64 "\n", ns(resolution));
    for (unsigned i = 1; i <= SAMPLE_COUNT; ++i)
    {
        uint64_t deadline;
        CHECK(pc_deadline(origin, (uint64_t)i * SAMPLE_PERIOD_NS, &deadline) ==
              0);
        struct timespec wake;
        struct timespec dispatch;
        struct timespec end;
        (void)sleep_until(deadline);
        CHECK(clock_gettime(CLOCK_MONOTONIC, &wake) == 0);
        CHECK(clock_gettime(CLOCK_MONOTONIC, &dispatch) == 0);
        volatile unsigned sum = 0;
        for (unsigned k = 0; k < BUSY_LOOP_ITERATIONS; ++k)
        {
            sum += k;
        }
        CHECK(sum == BUSY_LOOP_SUM &&
              clock_gettime(CLOCK_MONOTONIC, &end) == 0);
        printf("PC_SAMPLE planned=%" PRIu64 " wake=%" PRIu64
               " dispatch=%" PRIu64 " done=%" PRIu64 " late=%" PRIu64 "\n",
               deadline, ns(wake), ns(dispatch), ns(end),
               ns(wake) > deadline ? ns(wake) - deadline : 0);
    }
    CHECK(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_end) == 0);
    printf("PC_CPU elapsed=%" PRIu64
           " bound_claim=none load=uncontrolled_shared_host\n",
           ns(cpu_end) - ns(cpu_start));
}

/* A SIGALRM interrupts the sleep; resuming against the same absolute
 * deadline loses no time, whereas a relative restart extends the contract
 * by the time already slept. The previous
 * disposition is restored before returning. */
static void interrupted_sleep(void)
{
    struct sigaction action = {0};
    struct sigaction old;
    action.sa_handler = caught;
    CHECK(sigemptyset(&action.sa_mask) == 0 &&
          sigaction(SIGALRM, &action, &old) == 0);
    struct itimerval timer = {{0, 0}, {0, TIMER_DELAY_US}};
    struct itimerval off = {{0, 0}, {0, 0}};
    struct timespec now;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
    uint64_t deadline;
    CHECK(pc_deadline(ns(now), INTERRUPTED_SLEEP_NS, &deadline) == 0);
    CHECK(setitimer(ITIMER_REAL, &timer, NULL) == 0);
    int interrupted = sleep_until(deadline);
    CHECK(signals > 0 && setitimer(ITIMER_REAL, &off, NULL) == 0 &&
          sigaction(SIGALRM, &old, NULL) == 0);
    CHECK(clock_gettime(CLOCK_MONOTONIC, &now) == 0 && ns(now) >= deadline);
    printf(
        "PC_INTERRUPT delivered=%d sleep_eintr=%d absolute_deadline_unchanged=%" PRIu64
        "\n",
        (int)signals, interrupted, deadline);
}

int main(void)
{
    models();
    limits();
    monotonic_samples();
    interrupted_sleep();
    puts("PC_TIMING_PASS");
    return 0;
}
