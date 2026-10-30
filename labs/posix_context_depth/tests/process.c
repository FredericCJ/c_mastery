#define _GNU_SOURCE
#include "context.h"
#include "test.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

enum
{
    PATH_BYTES = 4096,
    OUTPUT_BYTES = 8192,
    WORKER_THREADS = 2,
    CHILD_PRIVATE_VALUE = 9,
    PARENT_VALUE = 7,
    SHARED_PARENT_WRITE = 1,
    SHARED_CHILD_WRITE = 5,
    CHILD_WRITE_FAILED_STATUS = 99,
    MISSING_CONFIG_STATUS = 23
};

/* Identity a thread or process reports about itself. */
struct pc_identity
{
    pid_t pid;
    pid_t tid;
    uid_t uid;
    gid_t gid;
    int private_value;
    uintptr_t private_address;
    uintptr_t shared_address;
};

/* private_integer is copy-on-write after fork; shared_byte is MAP_SHARED, so
 * parent and forked child see each other's writes to it. */
static int private_integer = PARENT_VALUE;
static unsigned char *shared_byte;

/* Worker threads park on a condition variable while the main thread forks and
 * launches, so every fork below happens in a multithreaded process. */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int ready;
static int release_workers;

static void *worker(void *argument)
{
    struct pc_identity *report = argument;
    report->pid = getpid();
    report->tid = (pid_t)syscall(SYS_gettid);
    report->uid = geteuid();
    report->gid = getegid();
    report->private_address = (uintptr_t)&private_integer;
    report->shared_address = (uintptr_t)shared_byte;
    CHECK(pthread_mutex_lock(&mutex) == 0);
    ++ready;
    CHECK(pthread_cond_broadcast(&cond) == 0);
    while (!release_workers)
    {
        CHECK(pthread_cond_wait(&cond, &mutex) == 0);
    }
    CHECK(pthread_mutex_unlock(&mutex) == 0);
    return NULL;
}

/* Starts the workers and returns with `mutex` HELD once all are parked. The
 * caller must call finish_workers() to release and join them. */
static void start_workers(pthread_t threads[WORKER_THREADS],
                          struct pc_identity reports[WORKER_THREADS])
{
    ready = 0;
    release_workers = 0;
    for (int i = 0; i < WORKER_THREADS; ++i)
    {
        CHECK(pthread_create(&threads[i], NULL, worker, &reports[i]) == 0);
    }
    CHECK(pthread_mutex_lock(&mutex) == 0);
    while (ready < WORKER_THREADS)
    {
        CHECK(pthread_cond_wait(&cond, &mutex) == 0);
    }
}

/* Releases the parked workers (mutex must be held) and joins them. */
static void finish_workers(pthread_t threads[WORKER_THREADS])
{
    release_workers = 1;
    CHECK(pthread_cond_broadcast(&cond) == 0);
    CHECK(pthread_mutex_unlock(&mutex) == 0);
    for (int i = 0; i < WORKER_THREADS; ++i)
    {
        CHECK(pthread_join(threads[i], NULL) == 0);
    }
}

static void read_exact(int fd, void *buffer, size_t length)
{
    unsigned char *bytes = buffer;
    size_t used = 0;
    while (used < length)
    {
        ssize_t got = read(fd, bytes + used, length - used);
        if (got < 0 && errno == EINTR)
        {
            continue;
        }
        CHECK(got > 0);
        used += (size_t)got;
    }
}

/* Forked child of identity(): modifies both variables, reports what it sees
 * through `channel_write_fd`, and exits. Calls no pthread function, because
 * the parent holds `mutex` across the fork and only the forking thread exists
 * here. */
static void identity_child(int channel_write_fd, unsigned char *shared)
{
    private_integer = CHILD_PRIVATE_VALUE;
    *shared = SHARED_CHILD_WRITE;
    struct pc_identity self = {getpid(),         0,
                               geteuid(),        getegid(),
                               private_integer,  (uintptr_t)&private_integer,
                               (uintptr_t)shared};
    const unsigned char *bytes = (const unsigned char *)&self;
    size_t used = 0;
    while (used < sizeof self)
    {
        ssize_t written =
            write(channel_write_fd, bytes + used, sizeof self - used);
        if (written > 0)
        {
            used += (size_t)written;
        }
        else if (written < 0 && errno == EINTR)
        {
            continue;
        }
        else
        {
            _exit(CHILD_WRITE_FAILED_STATUS);
        }
    }
    _exit(0);
}

/* Same virtual addresses in a forked child do not mean the same contents:
 * the private variable diverges, the shared mapping does not. */
static void identity(void)
{
    pthread_t threads[WORKER_THREADS];
    struct pc_identity workers[WORKER_THREADS];
    struct pc_identity child;
    int channel[2];
    int status;
    unsigned char *shared = mmap(NULL, 1, PROT_READ | PROT_WRITE,
                                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    CHECK(shared != MAP_FAILED);
    *shared = SHARED_PARENT_WRITE;
    shared_byte = shared;
    CHECK(pipe2(channel, O_CLOEXEC) == 0);
    start_workers(threads, workers);
    /* The mutex remains locked across fork: child must never call pthread APIs. */
    pid_t pid = fork();
    CHECK(pid >= 0);
    if (pid == 0)
    {
        close(channel[0]);
        identity_child(channel[1], shared);
    }
    close(channel[1]);
    read_exact(channel[0], &child, sizeof child);
    close(channel[0]);
    CHECK(waitpid(pid, &status, 0) == pid && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    CHECK(private_integer == PARENT_VALUE &&
          child.private_value == CHILD_PRIVATE_VALUE &&
          *shared == SHARED_CHILD_WRITE && child.pid == pid && pid != getpid());
    CHECK(child.uid == geteuid() && child.gid == getegid());
    finish_workers(threads);
    CHECK(workers[0].pid == getpid() && workers[1].pid == getpid() &&
          workers[0].tid != workers[1].tid);
    CHECK(workers[0].uid == geteuid() && workers[1].gid == getegid());
    CHECK(child.private_address == (uintptr_t)&private_integer &&
          child.shared_address == (uintptr_t)shared &&
          workers[0].private_address == child.private_address &&
          workers[1].shared_address == child.shared_address);
    printf(
        "PC_IDENTITY parent=%ld child=%ld thread0=%ld thread1=%ld private_parent=%d private_child=%d shared=%u private_address=%p shared_address=%p child_private_address=%ju child_shared_address=%ju\n",
        (long)getpid(), (long)pid, (long)workers[0].tid, (long)workers[1].tid,
        private_integer, child.private_value, *shared, (void *)&private_integer,
        (void *)shared, (uintmax_t)child.private_address,
        (uintmax_t)child.shared_address);
    CHECK(munmap(shared, 1) == 0);
    shared_byte = NULL;
}

/* Reads the whole output pipe into `output` (NUL-terminated); returns the
 * byte count. The helper's output must fit the buffer. */
static size_t drain_output(int fd, char output[OUTPUT_BYTES])
{
    size_t used = 0;
    for (;;)
    {
        ssize_t got = read(fd, output + used, OUTPUT_BYTES - 1 - used);
        if (got < 0 && errno == EINTR)
        {
            continue;
        }
        CHECK(got >= 0);
        if (!got)
        {
            return used;
        }
        used += (size_t)got;
        CHECK(used < OUTPUT_BYTES - 1);
    }
}

/* Which helper scenario launch_case() runs. */
enum launch_scenario
{
    LAUNCH_NORMAL,
    LAUNCH_MISSING_ENV,
    LAUNCH_MISSING_IMAGE
};

/* Checks the output a normally launched helper must have produced. */
static void check_helper_output(const char *output, const char *dir,
                                const struct rlimit *before,
                                const struct pc_launch *result)
{
    char expected[128];
    CHECK(strstr(output, "PC_HELPER") && strstr(output, dir));
    snprintf(expected, sizeof expected, "soft=%llu hard=%llu",
             (unsigned long long)before->rlim_cur,
             (unsigned long long)before->rlim_max);
    CHECK(strstr(output, expected));
    snprintf(expected, sizeof expected, "pid=%ld euid=%lu egid=%lu",
             (long)result->pid, (unsigned long)geteuid(),
             (unsigned long)getegid());
    CHECK(strstr(output, expected));
}

/* Runs the helper through pc_launch (spawn == 0) or pc_spawn and checks the
 * outcome, then that the parent's cwd and RLIMIT_NOFILE are unchanged. */
static void launch_case(const char *helper, int spawn,
                        enum launch_scenario scenario, const char *dir)
{
    char parent_cwd[PATH_BYTES];
    char after_cwd[PATH_BYTES];
    char output[OUTPUT_BYTES] = {0};
    int output_pipe[2];
    struct pc_launch result = {0};
    struct rlimit before;
    struct rlimit end;
    char *args[] = {(char *)helper, "", "with spaces", NULL};
    char *env[] = {"PC_CONFIG=synthetic=alpha", "PC_SECOND=beta", NULL};
    char *env_without_config[] = {"PC_SECOND=beta", NULL};
    CHECK(getcwd(parent_cwd, sizeof parent_cwd) &&
          getrlimit(RLIMIT_NOFILE, &before) == 0);
    int cwd = open(dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    CHECK(cwd >= 0 && pipe2(output_pipe, O_CLOEXEC) == 0);
    int (*launcher)(const char *, char *const[], char *const[], int, int,
                    struct pc_launch *) = spawn ? pc_spawn : pc_launch;
    const char *image = scenario == LAUNCH_MISSING_IMAGE
                            ? "/cmastery/no/such/original-helper"
                            : helper;
    char *const *environment =
        scenario == LAUNCH_MISSING_ENV ? env_without_config : env;
    CHECK(launcher(image, args, environment, cwd, output_pipe[1], &result) ==
          0);
    close(output_pipe[1]);
    size_t used = drain_output(output_pipe[0], output);
    close(output_pipe[0]);
    close(cwd);
    if (scenario == LAUNCH_MISSING_IMAGE)
    {
        CHECK(result.kind == 2 && result.value == ENOENT);
    }
    else if (scenario == LAUNCH_MISSING_ENV)
    {
        CHECK(result.kind == 1 && result.value == MISSING_CONFIG_STATUS &&
              strstr(output, "PC_MISSING_CONFIG"));
    }
    else
    {
        CHECK(result.kind == 1 && result.value == 0);
        check_helper_output(output, dir, &before, &result);
    }
    CHECK(getcwd(after_cwd, sizeof after_cwd) &&
          !strcmp(parent_cwd, after_cwd) &&
          getrlimit(RLIMIT_NOFILE, &end) == 0 &&
          before.rlim_cur == end.rlim_cur && before.rlim_max == end.rlim_max);
    printf("PC_LAUNCH backend=%s missing=%d bad=%d kind=%d value=%d %s",
           spawn ? "spawn" : "fork", scenario == LAUNCH_MISSING_ENV,
           scenario == LAUNCH_MISSING_IMAGE, result.kind, result.value, output);
    if (!used)
    {
        putchar('\n');
    }
}

/* Descriptor semantics: a dup shares the open file description (offset,
 * status flags) but not the descriptor flags; a second open() shares
 * neither. */
static void offsets(const char *dir)
{
    char path[PATH_BYTES];
    char first;
    char second;
    char third;
    char positioned;
    snprintf(path, sizeof path, "%s/offsets", dir);
    int original = open(path, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
    CHECK(original >= 0 && write(original, "abcdef", 6) == 6 &&
          lseek(original, 0, SEEK_SET) == 0);
    int duplicate = dup(original);
    int independent = open(path, O_RDONLY | O_CLOEXEC);
    CHECK(duplicate >= 0 && independent >= 0);
    CHECK(read(original, &first, 1) == 1 && first == 'a' &&
          lseek(duplicate, 0, SEEK_CUR) == 1);
    CHECK(read(duplicate, &second, 1) == 1 && second == 'b' &&
          lseek(original, 0, SEEK_CUR) == 2);
    /* pread() uses an explicit offset and leaves the shared one alone. */
    CHECK(pread(duplicate, &positioned, 1, 4) == 1 && positioned == 'e' &&
          lseek(original, 0, SEEK_CUR) == 2);
    CHECK(read(original, &third, 1) == 1 && third == 'c' &&
          lseek(duplicate, 0, SEEK_CUR) == 3);
    CHECK(read(independent, &first, 1) == 1 && first == 'a' &&
          lseek(duplicate, 0, SEEK_CUR) == 3);
    CHECK(fcntl(original, F_GETFD) & FD_CLOEXEC);
    CHECK(!(fcntl(duplicate, F_GETFD) & FD_CLOEXEC));
    int flags = fcntl(duplicate, F_GETFL);
    CHECK(flags >= 0 && fcntl(duplicate, F_SETFL, flags | O_NONBLOCK) == 0 &&
          (fcntl(original, F_GETFL) & O_NONBLOCK));
    CHECK(!(fcntl(independent, F_GETFL) & O_NONBLOCK));
    close(original);
    CHECK(lseek(duplicate, 0, SEEK_CUR) == 3);
    close(duplicate);
    close(independent);
    CHECK(unlink(path) == 0);
    puts(
        "PC_OFFSETS abc positioned=e duplicate=3 independent=1 cloexec_separate status_shared");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    char dir[] = "/tmp/cmastery-pc-process-XXXXXX";
    CHECK(mkdtemp(dir));
    identity();
    pthread_t threads[WORKER_THREADS];
    struct pc_identity reports[WORKER_THREADS];
    start_workers(threads, reports);
    for (int spawn = 0; spawn < 2; ++spawn)
    {
        launch_case(argv[1], spawn, LAUNCH_NORMAL, dir);
        launch_case(argv[1], spawn, LAUNCH_MISSING_ENV, dir);
        launch_case(argv[1], spawn, LAUNCH_MISSING_IMAGE, dir);
    }
    finish_workers(threads);
    offsets(dir);
    CHECK(rmdir(dir) == 0);
    puts("PC_PROCESS_PASS");
    return 0;
}
