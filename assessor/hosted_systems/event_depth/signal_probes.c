#include "signals.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/signalfd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(c)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(c))                                                      \
        {                                                              \
            fprintf(stderr, "SIGNAL-PROBE-CHECK:%s:%d errno=%d\n", #c, \
                    __LINE__, errno);                                  \
            return 1;                                                  \
        }                                                              \
    } while (0)

static int read_exact(int fd, void *data, size_t length)
{
    unsigned char *p = data;
    while (length)
    {
        struct pollfd f = {fd, POLLIN, 0};
        int ready;
        do
        {
            ready = poll(&f, 1, 3000);
        } while (ready < 0 && errno == EINTR);
        if (ready != 1)
            return -1;
        ssize_t n = read(fd, p, length);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            return -1;
        p += n;
        length -= (size_t)n;
    }
    return 0;
}

static int observe_syscall(pid_t child, long number, int fd)
{
    char name[80];
    snprintf(name, sizeof name, "/proc/%ld/syscall", (long)child);
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    do
    {
        FILE *f = fopen(name, "r");
        if (!f)
            return -1;
        long n = -999;
        unsigned long first = 0;
        int fields = fscanf(f, "%ld %lx", &n, &first);
        fclose(f);
        if (fields >= 1 && n == number &&
            (fd < 0 || (fields == 2 && first == (unsigned)fd)))
            return 0;
        sched_yield();
        clock_gettime(CLOCK_MONOTONIC, &now);
    } while (now.tv_sec - start.tv_sec < 3);
    return -1;
}

static _Atomic int ack_writer = -1;
static volatile sig_atomic_t got_signal;

static void acknowledge(int sig)
{
    int saved = errno;
    got_signal = sig;
    int fd = ack_writer;
    if (fd >= 0)
    {
        unsigned char b = 1;
        ssize_t n = write(fd, &b, 1);
        (void)n;
    }
    errno = saved;
}

static int coalesce(void)
{
    sigset_t set, old, pending;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    CHECK(pthread_sigmask(SIG_BLOCK, &set, &old) == 0);
    CHECK(kill(getpid(), SIGUSR1) == 0 && kill(getpid(), SIGUSR1) == 0);
    CHECK(sigpending(&pending) == 0 && sigismember(&pending, SIGUSR1) == 1);
    struct timespec zero = {0, 0};
    CHECK(sigtimedwait(&set, 0, &zero) == SIGUSR1);
    CHECK(sigtimedwait(&set, 0, &zero) == -1 && errno == EAGAIN);
    CHECK(pthread_sigmask(SIG_SETMASK, &old, 0) == 0);
    /* Original two-cell counted queue: unlike a pending bit, two admitted
     * records remain two records; a third is rejected before publication. */
    struct es_counted queue = {0};
    unsigned value = 99;
    CHECK(es_counted_push(&queue, 7) == 1 && es_counted_push(&queue, 9) == 1);
    CHECK(es_counted_push(&queue, 8) == 0 && queue.count == 2);
    CHECK(es_counted_pop(&queue, &value) == 1 && value == 7);
    CHECK(es_counted_pop(&queue, &value) == 1 && value == 9);
    CHECK(es_counted_pop(&queue, &value) == 0 && value == 9);
    puts(
        "COALESCE:generated=2;accepted=1;application_records=2;full_reject=1;");
    return 0;
}

static int realtime(void)
{
    int sig = SIGRTMIN;
    CHECK(sig <= SIGRTMAX);
    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, sig);
    CHECK(pthread_sigmask(SIG_BLOCK, &set, &old) == 0);
    int values[] = {7, 9, 8, 9};
    for (unsigned i = 0; i < 4; ++i)
        CHECK(sigqueue(getpid(), sig, (union sigval){.sival_int = values[i]}) ==
              0);
    for (unsigned i = 0; i < 4; ++i)
    {
        siginfo_t info;
        CHECK(sigtimedwait(&set, &info, &(struct timespec){0, 0}) == sig);
        CHECK(info.si_code == SI_QUEUE && info.si_value.sival_int == values[i]);
    }
    struct rlimit original, limited;
    CHECK(getrlimit(RLIMIT_SIGPENDING, &original) == 0);
    limited = original;
    limited.rlim_cur = 0;
    CHECK(setrlimit(RLIMIT_SIGPENDING, &limited) == 0);
    int r = sigqueue(getpid(), sig, (union sigval){.sival_int = 5}), e = errno;
    CHECK(setrlimit(RLIMIT_SIGPENDING, &original) == 0);
    CHECK(r == -1 && e == EAGAIN);
    CHECK(pthread_sigmask(SIG_SETMASK, &old, 0) == 0);
    puts("REALTIME:accepted=4;ordered_payloads=7,9,8,9;child_soft_limit_zero="
         "EAGAIN;restored=1;not_host_queue_capacity=1;");
    return 0;
}

static int restart(int restart)
{
    int input[2], acks[2], result[2];
    CHECK(pipe(input) == 0 && pipe(acks) == 0 && pipe(result) == 0);
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        alarm(6);
        close(input[1]);
        close(acks[0]);
        close(result[0]);
        ack_writer = acks[1];
        struct sigaction a = {0};
        a.sa_handler = acknowledge;
        sigemptyset(&a.sa_mask);
        a.sa_flags = restart ? SA_RESTART : 0;
        if (sigaction(SIGUSR1, &a, 0))
            _exit(2);
        unsigned char byte = 0;
        errno = 0;
        ssize_t n = read(input[0], &byte, 1);
        int record[4] = {(int)n, errno, (int)got_signal, byte};
        if (write(result[1], record, sizeof record) != (ssize_t)sizeof record)
            _exit(3);
        close(input[0]);
        close(acks[1]);
        close(result[1]);
        _exit(0);
    }
    close(input[0]);
    close(acks[1]);
    close(result[1]);
    CHECK(observe_syscall(child, SYS_read, input[0]) == 0);
    CHECK(kill(child, SIGUSR1) == 0);
    unsigned char ack;
    CHECK(read_exact(acks[0], &ack, 1) == 0);
    if (restart)
    {
        CHECK(observe_syscall(child, SYS_read, input[0]) == 0);
        CHECK(write(input[1], "X", 1) == 1);
    }
    int record[4], status;
    CHECK(read_exact(result[0], record, sizeof record) == 0);
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    CHECK(record[0] == (restart ? 1 : -1) && record[2] == SIGUSR1);
    CHECK(restart ? record[3] == 'X' : record[1] == EINTR);
    close(input[1]);
    close(acks[0]);
    close(result[0]);
    printf("RESTART:flag=%d;return=%d;errno=%d;handler=%d;byte=%d;enrollment="
           "proc_syscall;\n",
           restart, record[0], record[1], record[2], record[3]);
    return 0;
}

static int partial(void)
{
    int fd[2];
    CHECK(pipe(fd) == 0);
    CHECK(write(fd[1], "AB", 2) == 2);
    char b[8] = {0};
    CHECK(read(fd[0], b, sizeof b) == 2 && b[0] == 'A' && b[1] == 'B');
    close(fd[0]);
    close(fd[1]);
    puts("PARTIAL:requested=8;completed=2;cursor=2;not_an_EINTR_result=1;");
    return 0;
}

static int lost(int repaired)
{
    int commands[2], reports[2];
    CHECK(pipe(commands) == 0 && pipe(reports) == 0);
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        alarm(6);
        close(commands[1]);
        close(reports[0]);
        ack_writer = repaired ? -1 : reports[1];
        got_signal = 0;
        struct sigaction a = {0};
        a.sa_handler = acknowledge;
        sigemptyset(&a.sa_mask);
        if (sigaction(SIGUSR1, &a, 0) || sigaction(SIGUSR2, &a, 0))
            _exit(2);
        sigset_t set, old;
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        if (repaired && pthread_sigmask(SIG_BLOCK, &set, &old))
            _exit(3);
        if (got_signal)
            _exit(4); /* check false */
        if (write(reports[1], "C", 1) != 1)
            _exit(5);
        char command;
        if (read_exact(commands[0], &command, 1))
            _exit(6);
        if (repaired)
        {
            sigset_t waitmask = old;
            sigdelset(&waitmask, SIGUSR1);
            if (sigsuspend(&waitmask) != -1 || errno != EINTR ||
                got_signal != SIGUSR1)
                _exit(7);
            if (pthread_sigmask(SIG_SETMASK, &old, 0))
                _exit(8);
        }
        else
        {
            /* Parent already observed the handler before authorizing this pause. */
            if (got_signal != SIGUSR1)
                _exit(9);
            if (write(reports[1], "B", 1) != 1)
                _exit(10);
            pause(); /* intentionally defective teaching specimen; rescue is SIGUSR2
                      */
            if (got_signal != SIGUSR2)
                _exit(11);
        }
        close(commands[0]);
        close(reports[1]);
        _exit(0);
    }
    close(commands[0]);
    close(reports[1]);
    char b;
    CHECK(read_exact(reports[0], &b, 1) == 0 && b == 'C');
    CHECK(kill(child, SIGUSR1) == 0);
    if (!repaired)
        CHECK(read_exact(reports[0], &b, 1) == 0 && b == 1);
    CHECK(write(commands[1], "G", 1) == 1);
    if (!repaired)
    {
        CHECK(read_exact(reports[0], &b, 1) == 0 && b == 'B');
        CHECK(observe_syscall(child, SYS_pause, -1) == 0);
        CHECK(kill(child, SIGUSR2) == 0);
    }
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    close(commands[1]);
    close(reports[0]);
    printf(
        "LOST-WAKE:repaired=%d;predicate_set_before_wait=1;rescued_pause=%d;\n",
        repaired, !repaired);
    return 0;
}

static int descriptor(void)
{
    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    CHECK(pthread_sigmask(SIG_BLOCK, &set, &old) == 0);
    int fd = signalfd(-1, &set, SFD_CLOEXEC | SFD_NONBLOCK);
    CHECK(fd >= 0);
    CHECK(kill(getpid(), SIGUSR1) == 0 && kill(getpid(), SIGUSR2) == 0);
    struct signalfd_siginfo info[2];
    ssize_t n = read(fd, info, sizeof info);
    CHECK(n == (ssize_t)sizeof info);
    unsigned seen = 0;
    for (unsigned i = 0; i < 2; ++i)
    {
        CHECK(info[i].ssi_signo == SIGUSR1 || info[i].ssi_signo == SIGUSR2);
        seen |= info[i].ssi_signo == SIGUSR1 ? 1 : 2;
    }
    CHECK(seen == 3);
    CHECK(read(fd, info, sizeof info) == -1 && errno == EAGAIN);
    close(fd);
    CHECK(pthread_sigmask(SIG_SETMASK, &old, 0) == 0);
    puts(
        "SIGNALFD:records=2;predicates=3;empty=EAGAIN;blocked_mask_restored=1;");
    return 0;
}

struct rd_workers
{
    pthread_mutex_t lock;
    pthread_cond_t cv;
    unsigned ready, finished;
    int error;
    char *owned;
    sigset_t set;
};

static void *worker(void *arg)
{
    struct rd_workers *w = arg;
    sigset_t current;
    int e = pthread_sigmask(SIG_SETMASK, 0, &current);
    pthread_mutex_lock(&w->lock);
    if (e || sigismember(&current, SIGUSR1) != 1 ||
        sigismember(&current, SIGUSR2) != 1)
        w->error = 1;
    ++w->ready;
    pthread_cond_broadcast(&w->cv);
    while (!w->finished)
        pthread_cond_wait(&w->cv, &w->lock);
    pthread_mutex_unlock(&w->lock);
    return 0;
}

static void *receiver(void *arg)
{
    struct rd_workers *w = arg;
    siginfo_t info;
    int sig = sigtimedwait(&w->set, &info, &(struct timespec){2, 0});
    /* Synchronous acceptance is ordinary thread execution: malloc/stdio/mutex
     * are permitted by their ordinary contracts, unlike handler invocation. */
    char *value = malloc(32);
    if (value)
        snprintf(value, 32, "accepted:%d", sig);
    pthread_mutex_lock(&w->lock);
    if (sig != SIGUSR1 || !value)
        w->error = 1;
    w->owned = value;
    w->finished = 1;
    pthread_cond_broadcast(&w->cv);
    pthread_mutex_unlock(&w->lock);
    return 0;
}

static int threads(void)
{
    struct rd_workers w = {.lock = PTHREAD_MUTEX_INITIALIZER,
                           .cv = PTHREAD_COND_INITIALIZER};
    sigemptyset(&w.set);
    sigaddset(&w.set, SIGUSR1);
    sigaddset(&w.set, SIGUSR2);
    sigset_t old;
    CHECK(pthread_sigmask(SIG_BLOCK, &w.set, &old) == 0);
    pthread_t a, b, r, plugin;
    CHECK(pthread_create(&a, 0, worker, &w) == 0 &&
          pthread_create(&b, 0, worker, &w) == 0);
    /* This is the plugin contract: creation is performed by a masked thread;
     * entry verifies inherited masks before enrolling in ordinary work. */
    CHECK(pthread_create(&plugin, 0, worker, &w) == 0);
    pthread_mutex_lock(&w.lock);
    while (w.ready < 3)
        pthread_cond_wait(&w.cv, &w.lock);
    pthread_mutex_unlock(&w.lock);
    CHECK(pthread_create(&r, 0, receiver, &w) == 0);
    CHECK(kill(getpid(), SIGUSR1) == 0);
    CHECK(pthread_join(r, 0) == 0 && pthread_join(a, 0) == 0 &&
          pthread_join(b, 0) == 0 && pthread_join(plugin, 0) == 0);
    CHECK(!w.error && w.owned && strncmp(w.owned, "accepted:", 9) == 0);
    free(w.owned);
    w.owned = 0;
    CHECK(pthread_cond_destroy(&w.cv) == 0 &&
          pthread_mutex_destroy(&w.lock) == 0);
    CHECK(pthread_sigmask(SIG_SETMASK, &old, 0) == 0);
    puts("THREADS:workers=2;plugin=1;inherited_masks=3;receiver=1;owned_"
         "allocation_freed=1;joined=4;");
    return 0;
}

static void *unmasked_plugin(void *arg)
{
    int *pipes = arg;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    if (pthread_sigmask(SIG_UNBLOCK, &set, 0))
        return (void *)1;
    if (write(pipes[1], "R", 1) != 1)
        return (void *)1;
    char stop;
    if (read_exact(pipes[2], &stop, 1))
        return (void *)1;
    if (pthread_sigmask(SIG_BLOCK, &set, 0))
        return (void *)1;
    return 0;
}

static int bad_plugin(void)
{
    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    CHECK(pthread_sigmask(SIG_BLOCK, &set, &old) == 0);
    int reports[2], commands[2];
    CHECK(pipe(reports) == 0 && pipe(commands) == 0);
    ack_writer = reports[1];
    struct sigaction a = {0}, previous;
    a.sa_handler = acknowledge;
    sigemptyset(&a.sa_mask);
    CHECK(sigaction(SIGUSR1, &a, &previous) == 0);
    int context[] = {reports[0], reports[1], commands[0]};
    pthread_t thread;
    CHECK(pthread_create(&thread, 0, unmasked_plugin, context) == 0);
    char b;
    CHECK(read_exact(reports[0], &b, 1) == 0 && b == 'R');
    CHECK(kill(getpid(), SIGUSR1) == 0);
    CHECK(read_exact(reports[0], &b, 1) == 0 && b == 1);
    CHECK(write(commands[1], "Q", 1) == 1);
    void *out;
    CHECK(pthread_join(thread, &out) == 0 && !out);
    CHECK(sigtimedwait(&set, 0, &(struct timespec){0, 0}) == -1 &&
          errno == EAGAIN);
    CHECK(sigaction(SIGUSR1, &previous, 0) == 0);
    CHECK(pthread_sigmask(SIG_SETMASK, &old, 0) == 0);
    close(reports[0]);
    close(reports[1]);
    close(commands[0]);
    close(commands[1]);
    ack_writer = -1;
    puts("BAD-PLUGIN:sole_unmasked_worker_received_process_signal=1;receiver_"
         "pending=0;no_delivery_to_external_process=1;");
    return 0;
}

static int children(int late)
{
    struct es_signals p = {0};
    CHECK(es_open(&p) == 0);
    pid_t ids[3] = {0};
    int statuses[3] = {0};
    int gate[2];
    CHECK(pipe(gate) == 0);
    for (unsigned i = 0; i < 3; ++i)
    {
        ids[i] = fork();
        CHECK(ids[i] >= 0);
        if (!ids[i])
        {
            alarm(6);
            close(gate[1]);
            if (late && i == 2)
            {
                char c;
                if (read_exact(gate[0], &c, 1))
                    _exit(99);
            }
            close(gate[0]);
            _exit((int)(20 + i));
        }
    }
    close(gate[0]);
    /* WNOWAIT observes exit without reaping, so all ready children are known
     * before one WNOHANG sweep. It does not consume a SIGCHLD notification. */
    for (unsigned i = 0; i < (late ? 2u : 3u); ++i)
    {
        siginfo_t info;
        CHECK(waitid(P_PID, (id_t)ids[i], &info, WEXITED | WNOWAIT) == 0);
    }
    CHECK(kill(getpid(), SIGUSR1) == 0);
    int n = es_reap(ids, statuses, 3);
    CHECK(n == (late ? 2 : 3));
    if (late)
    {
        CHECK(write(gate[1], "X", 1) == 1);
        siginfo_t info;
        CHECK(waitid(P_PID, (id_t)ids[2], &info, WEXITED | WNOWAIT) == 0);
    }
    close(gate[1]);
    unsigned seen = 0;
    for (unsigned i = 0;
         i < 5 && ((seen & ES_STOP) == 0 || (seen & ES_CHILD) == 0); ++i)
    {
        CHECK(es_wait(&p, &(struct timespec){1, 0}) == 1);
        seen |= es_take(&p, 7);
    }
    CHECK((seen & (ES_STOP | ES_CHILD)) == (ES_STOP | ES_CHILD));
    if (late)
        CHECK(es_reap(ids, statuses, 3) == 1);
    CHECK(es_reap(ids, statuses, 3) == 0);
    for (unsigned i = 0; i < 3; ++i)
        CHECK(ids[i] == 0 && WIFEXITED(statuses[i]) &&
              WEXITSTATUS(statuses[i]) == (int)(20 + i));
    CHECK(es_close(&p) == 0);
    printf(
        "CHILDREN:late=%d;reaped=3;shutdown=1;duplicate_reap=0;owned_only=1;\n",
        late);
    return 0;
}

static int deadline(void)
{
    int input[2], acks[2];
    CHECK(pipe(input) == 0 && pipe(acks) == 0);
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        alarm(6);
        close(input[1]);
        close(acks[0]);
        ack_writer = acks[1];
        struct sigaction a = {0};
        a.sa_handler = acknowledge;
        sigemptyset(&a.sa_mask);
        if (sigaction(SIGUSR1, &a, 0))
            _exit(2);
        struct timespec d;
        if (clock_gettime(CLOCK_MONOTONIC, &d))
            _exit(3);
        d.tv_sec += 2;
        int r = es_wait_fd(input[0], d);
        close(input[0]);
        close(acks[1]);
        _exit(r == 0 ? 0 : 4);
    }
    close(input[0]);
    close(acks[1]);
    for (unsigned i = 0; i < 3; ++i)
    {
        CHECK(observe_syscall(child, SYS_ppoll, -1) == 0);
        CHECK(kill(child, SIGUSR1) == 0);
        char b;
        CHECK(read_exact(acks[0], &b, 1) == 0);
    }
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    close(input[1]);
    close(acks[0]);
    puts("DEADLINE:real_ppoll_interruptions=3;absolute_timeout_observed=1;not_"
         "WCET=1;");
    return 0;
}

int main(int argc, char **argv)
{
    CHECK(atomic_is_lock_free(&ack_writer));
    alarm(15);
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "coalesce"))
        return coalesce();
    if (!strcmp(argv[1], "realtime"))
        return realtime();
    if (!strcmp(argv[1], "restart0"))
        return restart(0);
    if (!strcmp(argv[1], "restart1"))
        return restart(1);
    if (!strcmp(argv[1], "partial"))
        return partial();
    if (!strcmp(argv[1], "lost"))
        return lost(0);
    if (!strcmp(argv[1], "repaired"))
        return lost(1);
    if (!strcmp(argv[1], "signalfd"))
        return descriptor();
    if (!strcmp(argv[1], "threads"))
        return threads();
    if (!strcmp(argv[1], "bad-plugin"))
        return bad_plugin();
    if (!strcmp(argv[1], "children"))
        return children(0);
    if (!strcmp(argv[1], "late-child"))
        return children(1);
    if (!strcmp(argv[1], "deadline"))
        return deadline();
    return 2;
}
