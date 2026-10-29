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

static uint64_t ns(struct timespec t)
{
    CHECK(t.tv_sec >= 0 && (uint64_t)t.tv_sec <= UINT64_MAX / 1000000000 &&
          t.tv_nsec >= 0 && t.tv_nsec < 1000000000);
    return (uint64_t)t.tv_sec * 1000000000 + (uint64_t)t.tv_nsec;
}

static struct timespec ts(uint64_t n)
{
    struct timespec t = {(time_t)(n / 1000000000), (long)(n % 1000000000)};
    CHECK((uint64_t)t.tv_sec == n / 1000000000);
    return t;
}

static volatile sig_atomic_t signals;

static void caught(int sig)
{
    (void)sig;
    ++signals;
}

static void models(void)
{
    uint64_t d = 77;
    CHECK(pc_deadline(100, 20, &d) == 0 && d == 120);
    CHECK(pc_remaining(d, 105) == 15 && pc_remaining(d, 120) == 0 &&
          pc_remaining(d, 121) == 0);
    CHECK(pc_deadline(UINT64_MAX, 1, &d) == ERANGE && d == 120);
    CHECK(pc_deadline(UINT64_MAX, 0, &d) == 0 && d == UINT64_MAX);
    CHECK(pc_deadline(0, 0, NULL) == EINVAL);
    /* Same 15 active ticks + 100 suspended ticks; no host clock or suspend changed. */
    CHECK(pc_remaining(120, 115) == 5 && pc_remaining(120, 215) == 0);
    int64_t wall[] = {1000, 1005, 500, 9000};
    for (size_t i = 0; i < 4; ++i)
    {
        (void)wall[i];
        CHECK(pc_remaining(120, 100 + i * 5) == 20 - i * 5);
    }
    uint64_t planned[] = {5, 10, 20}, dispatch[] = {7, 13, 19};
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

static void limits(void)
{
    struct rlimit before, after;
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
    int p[2], status;
    CHECK(pipe(p) == 0);
    pid_t pid = fork();
    CHECK(pid >= 0);
    if (!pid)
    {
        close(p[0]);
        struct rlimit smaller = before;
        smaller.rlim_cur = before.rlim_cur > 32 ? 32 : before.rlim_cur;
        int ok = setrlimit(RLIMIT_NOFILE, &smaller) == 0;
        struct rlimit seen;
        if (getrlimit(RLIMIT_NOFILE, &seen) ||
            seen.rlim_cur != smaller.rlim_cur)
            ok = 0;
        ssize_t sent;
        do
        {
            sent = write(p[1], &ok, sizeof ok);
        } while (sent < 0 && errno == EINTR);
        _exit(ok && sent == (ssize_t)sizeof ok ? 0 : 98);
    }
    close(p[1]);
    int ok = 0;
    CHECK(read(p[0], &ok, sizeof ok) == sizeof ok && ok);
    close(p[0]);
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
    unsigned inspected_cpu = 3, available_now = 1;
    CHECK(pc_request(available_now, inspected_cpu, 0, &accepted) == EINVAL &&
          accepted == 3);
    puts(
        "PC_LIMIT child_lowered parent_unchanged errors=EPERM,EINVAL cpu_snapshot_stale=model");
}

static void samples(void)
{
    struct timespec start, cpu0, cpu1, res;
    CHECK(clock_getres(CLOCK_MONOTONIC, &res) == 0 &&
          clock_gettime(CLOCK_MONOTONIC, &start) == 0 &&
          clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu0) == 0);
    uint64_t origin = ns(start);
    printf("PC_RESOLUTION ns=%" PRIu64 "\n", ns(res));
    for (unsigned i = 1; i <= 12; ++i)
    {
        uint64_t deadline;
        CHECK(pc_deadline(origin, (uint64_t)i * 2000000, &deadline) == 0);
        struct timespec t = ts(deadline), wake, dispatch, end;
        int r;
        do
        {
            r = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
        } while (r == EINTR);
        CHECK(r == 0 && clock_gettime(CLOCK_MONOTONIC, &wake) == 0);
        CHECK(clock_gettime(CLOCK_MONOTONIC, &dispatch) == 0);
        volatile unsigned sum = 0;
        for (unsigned k = 0; k < 10000; ++k)
            sum += k;
        CHECK(sum == 49995000 && clock_gettime(CLOCK_MONOTONIC, &end) == 0);
        printf("PC_SAMPLE planned=%" PRIu64 " wake=%" PRIu64
               " dispatch=%" PRIu64 " done=%" PRIu64 " late=%" PRIu64 "\n",
               deadline, ns(wake), ns(dispatch), ns(end),
               ns(wake) > deadline ? ns(wake) - deadline : 0);
    }
    CHECK(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu1) == 0);
    printf("PC_CPU elapsed=%" PRIu64
           " bound_claim=none load=uncontrolled_shared_host\n",
           ns(cpu1) - ns(cpu0));
    struct sigaction action = {0}, old;
    action.sa_handler = caught;
    CHECK(sigemptyset(&action.sa_mask) == 0 &&
          sigaction(SIGALRM, &action, &old) == 0);
    struct itimerval timer = {{0, 0}, {0, 1000}}, off = {{0, 0}, {0, 0}};
    CHECK(clock_gettime(CLOCK_MONOTONIC, &start) == 0);
    uint64_t deadline;
    CHECK(pc_deadline(ns(start), 10000000, &deadline) == 0);
    struct timespec t = ts(deadline);
    CHECK(setitimer(ITIMER_REAL, &timer, NULL) == 0);
    int interrupted = 0, r;
    do
    {
        r = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
        if (r == EINTR)
            ++interrupted;
    } while (r == EINTR);
    CHECK(r == 0 && signals > 0 && setitimer(ITIMER_REAL, &off, NULL) == 0 &&
          sigaction(SIGALRM, &old, NULL) == 0);
    CHECK(clock_gettime(CLOCK_MONOTONIC, &start) == 0 && ns(start) >= deadline);
    printf(
        "PC_INTERRUPT delivered=%d sleep_eintr=%d absolute_deadline_unchanged=%" PRIu64
        "\n",
        (int)signals, interrupted, deadline);
}

int main(void)
{
    models();
    limits();
    samples();
    puts("PC_TIMING_PASS");
    return 0;
}
