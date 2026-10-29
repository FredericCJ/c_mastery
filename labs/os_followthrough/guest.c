#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "OF_GUEST_FAIL:%d:%s errno=%d\n", __LINE__, #x, \
                    errno);                                                 \
            exit(1);                                                        \
        }                                                                   \
    } while (0)
static const char *base = "/sys/fs/cgroup/of";

static uint64_t now(clockid_t id)
{
    struct timespec t;
    CHECK(!clock_gettime(id, &t));
    return (uint64_t)t.tv_sec * 1000000000u + (unsigned long)t.tv_nsec;
}

static void put(const char *dir, const char *file, const char *value)
{
    char path[512];
    CHECK(snprintf(path, sizeof path, "%s/%s", dir, file) > 0);
    int fd = open(path, O_WRONLY | O_CLOEXEC);
    CHECK(fd >= 0);
    CHECK(write(fd, value, strlen(value)) == (ssize_t)strlen(value) &&
          !close(fd));
}

static void show(const char *dir, const char *file)
{
    char path[512], buf[4096];
    CHECK(snprintf(path, sizeof path, "%s/%s", dir, file) > 0);
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0)
    {
        printf("OF_RAW %s unavailable errno=%d\n", path, errno);
        return;
    }
    printf("OF_RAW_BEGIN %s\n", path);
    ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0)
    {
        CHECK(fwrite(buf, 1, (size_t)n, stdout) == (size_t)n);
    }
    CHECK(n == 0 && !close(fd));
    printf("OF_RAW_END %s\n", path);
}

static void snapshot(const char *dir)
{
    const char *files[] = {"cgroup.procs",  "cpu.max",         "cpu.max.burst",
                           "cpu.weight",    "cpu.stat",        "cpu.stat.local",
                           "cpu.pressure",  "memory.max",      "memory.current",
                           "memory.events", "memory.pressure", "io.max",
                           "io.stat",       "io.pressure"};
    for (size_t i = 0; i < sizeof files / sizeof *files; i++)
    {
        show(dir, files[i]);
    }
}

static void wait_for(pid_t pid, unsigned seconds)
{
    uint64_t end = now(CLOCK_MONOTONIC) + (uint64_t)seconds * 1000000000u;
    int status;
    for (;;)
    {
        pid_t r = waitpid(pid, &status, WNOHANG);
        CHECK(r >= 0);
        if (r == pid)
        {
            printf("OF_EXIT pid=%ld raw=%d exited=%d code=%d signal=%d\n",
                   (long)pid, status, WIFEXITED(status),
                   WIFEXITED(status) ? WEXITSTATUS(status) : -1,
                   WIFSIGNALED(status) ? WTERMSIG(status) : 0);
            return;
        }
        if (now(CLOCK_MONOTONIC) > end)
        {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            fprintf(stderr, "OF_TIMEOUT pid=%ld\n", (long)pid);
            exit(1);
        }
        struct timespec nap = {0, 10000000};
        nanosleep(&nap, NULL);
    }
}

static void compute(unsigned memory)
{
    size_t size = 64u * 1024 * 1024;
    volatile unsigned char *data = NULL;
    if (memory)
    {
        data = malloc(size);
        if (!data)
        {
            printf("OF_ALLOCATION_FAILURE errno=%d\n", errno);
            _exit(12);
        }
        for (size_t i = 0; i < size; i += 4096)
        {
            data[i] = (unsigned char)(i / 4096);
        }
        printf("OF_ALLOCATED bytes=%zu\n", size);
    }
    uint64_t wall = now(CLOCK_MONOTONIC), cpu = now(CLOCK_PROCESS_CPUTIME_ID),
             work = 1, iterations = 0;
    do
    {
        for (unsigned i = 0; i < 65536; i++)
        {
            work = work * 1664525u + 1013904223u;
        }
        iterations += 65536;
    } while (now(CLOCK_MONOTONIC) - wall < 1000000000u);
    printf("OF_COMPUTE iterations=%" PRIu64 " checksum=%" PRIu64
           " wall_ns=%" PRIu64 " cpu_ns=%" PRIu64 "\n",
           iterations, work, now(CLOCK_MONOTONIC) - wall,
           now(CLOCK_PROCESS_CPUTIME_ID) - cpu);
    if (data)
    {
        unsigned checksum = 0;
        for (size_t i = 0; i < size; i += 4096)
        {
            checksum += data[i];
        }
        printf("OF_ALLOC_CHECKSUM %u\n", checksum);
    }
    free((void *)data);
}

static void io_work(const char *dir, unsigned sync_each, unsigned concurrent,
                    unsigned enospc)
{
    struct statvfs space;
    struct stat mount;
    CHECK(!statvfs(dir, &space) && !stat(dir, &mount));
    printf("OF_IO_PLACEMENT dir=%s device=%u:%u available=%" PRIu64
           " block=%lu\n",
           dir, major(mount.st_dev), minor(mount.st_dev),
           (uint64_t)space.f_bavail * space.f_frsize,
           (unsigned long)space.f_frsize);
    CHECK((uint64_t)space.f_bavail * space.f_frsize >=
          (enospc ? 1024u * 1024u : 8u * 1024u * 1024u));
    char path[256];
    CHECK(snprintf(path, sizeof path, "%s/original-%ld", dir, (long)getpid()) >
          0);
    int fd = open(path, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
    CHECK(fd >= 0);
    unsigned char buffer[4096];
    for (unsigned i = 0; i < sizeof buffer; i++)
    {
        buffer[i] = (unsigned char)(i % 251);
    }
    uint64_t wrote = 0, readbytes = 0, sum = 0, start = now(CLOCK_MONOTONIC),
             cpu = now(CLOCK_PROCESS_CPUTIME_ID);
    int error = 0;
    size_t wanted = 4u * 1024 * 1024;
    for (size_t at = 0; at < wanted;)
    {
        ssize_t n = write(fd, buffer, sizeof buffer);
        if (n < 0 && errno == EINTR)
        {
            continue;
        }
        if (n < 0)
        {
            error = errno;
            break;
        }
        CHECK(n > 0);
        at += (size_t)n;
        wrote += (size_t)n;
        if (sync_each)
        {
            CHECK(!fdatasync(fd));
        }
    }
    if (enospc)
    {
        CHECK(error == ENOSPC && wrote < wanted);
        printf("OF_ENOSPC accepted=%" PRIu64 " remaining=%zu errno=%d\n", wrote,
               wanted - (size_t)wrote, error);
    }
    else
    {
        CHECK(!error && wrote == wanted);
    }
    CHECK(!fsync(fd));
    uint64_t ack = wrote;
    CHECK(lseek(fd, 0, SEEK_SET) == 0);
    pid_t reader = -1;
    if (concurrent)
    {
        reader = fork();
        CHECK(reader >= 0);
        if (!reader)
        {
            uint64_t count = 0;
            ssize_t n;
            int independent = open(path, O_RDONLY | O_CLOEXEC);
            CHECK(independent >= 0);
            while ((n = read(independent, buffer, sizeof buffer)) > 0)
            {
                count += (size_t)n;
            }
            CHECK(n == 0 && !close(independent));
            printf("OF_OTHER_READER pid=%ld logical=%" PRIu64 "\n",
                   (long)getpid(), count);
            _exit(0);
        }
    }
    struct rusage before, after;
    CHECK(!getrusage(RUSAGE_SELF, &before));
    for (;;)
    {
        ssize_t n = read(fd, buffer, sizeof buffer);
        if (n < 0 && errno == EINTR)
        {
            continue;
        }
        CHECK(n >= 0);
        if (!n)
        {
            break;
        }
        readbytes += (size_t)n;
        for (ssize_t i = 0; i < n; i++)
        {
            sum += buffer[i];
        }
    }
    CHECK(!getrusage(RUSAGE_SELF, &after) && readbytes == wrote);
    uint64_t expected_sum = 0;
    for (uint64_t i = 0; i < wrote; i++)
    {
        expected_sum += (i % 4096) % 251;
    }
    CHECK(sum == expected_sum);
    if (reader > 0)
    {
        wait_for(reader, 5);
    }
    struct stat s;
    CHECK(!fstat(fd, &s));
    printf("OF_IO wrote=%" PRIu64 " read=%" PRIu64 " fsync_ack=%" PRIu64
           " size=%lld checksum=%" PRIu64 " wall_ns=%" PRIu64 " cpu_ns=%" PRIu64
           " major_delta=%ld minor_delta=%ld sync_each=%u concurrent=%u\n",
           wrote, readbytes, ack, (long long)s.st_size, sum,
           now(CLOCK_MONOTONIC) - start, now(CLOCK_PROCESS_CPUTIME_ID) - cpu,
           after.ru_majflt - before.ru_majflt,
           after.ru_minflt - before.ru_minflt, sync_each, concurrent);
    show("/proc/self", "io");
    show("/proc", "diskstats");
    CHECK(!close(fd) && !unlink(path));
}

static pid_t launch(const char *dir, unsigned mode, const char *io_dir,
                    unsigned sync_each, unsigned concurrent, unsigned enospc)
{
    int entered[2], start[2];
    CHECK(!pipe2(entered, O_CLOEXEC) && !pipe2(start, O_CLOEXEC));
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        close(entered[0]);
        close(start[1]);
        if (dir)
        {
            char pid[32];
            snprintf(pid, sizeof pid, "%ld", (long)getpid());
            put(dir, "cgroup.procs", pid);
        }
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(0, &set);
        CHECK(!sched_setaffinity(0, sizeof set, &set));
        show("/proc/self", "cgroup");
        CHECK(write(entered[1], "E", 1) == 1);
        close(entered[1]);
        char token;
        CHECK(read(start[0], &token, 1) == 1);
        close(start[0]);
        if (mode == 2)
        {
            io_work(io_dir, sync_each, concurrent, enospc);
        }
        else
        {
            if (mode == 3)
            {
                struct rlimit limit = {32u * 1024 * 1024, 32u * 1024 * 1024};
                CHECK(!setrlimit(RLIMIT_AS, &limit));
                puts("OF_RLIMIT_AS 33554432 worker-only");
            }
            compute(mode);
        }
        _exit(0);
    }
    close(entered[1]);
    close(start[0]);
    char token;
    CHECK(read(entered[0], &token, 1) == 1 && !close(entered[0]));
    char proc[64];
    snprintf(proc, sizeof proc, "/proc/%ld", (long)child);
    show(proc, "cgroup");
    CHECK(write(start[1], "S", 1) == 1 && !close(start[1]));
    return child;
}

static void trial(const char *name, const char *quota, const char *memory,
                  unsigned mode)
{
    char dir[256];
    snprintf(dir, sizeof dir, "%s/%s", base, name);
    CHECK(!mkdir(dir, 0700));
    put(dir, "cpu.max", quota);
    put(dir, "memory.max", memory);
    printf("OF_TRIAL_BEGIN %s\n", name);
    snapshot(dir);
    pid_t child = launch(dir, mode, NULL, 0, 0, 0);
    struct timespec nap = {0, 100000000};
    nanosleep(&nap, NULL);
    char proc[64];
    snprintf(proc, sizeof proc, "/proc/%ld", (long)child);
    show(proc, "status");
    show(dir, "memory.current");
    wait_for(child, 8);
    snapshot(dir);
    CHECK(!rmdir(dir));
    printf("OF_TRIAL_END %s cleanup=1\n", name);
}

static void namespace_probe(void)
{
    struct stat parent_pid, parent_mnt, after_pid, after_mnt;
    CHECK(!stat("/proc/self/ns/pid", &parent_pid) &&
          !stat("/proc/self/ns/mnt", &parent_mnt));
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        CHECK(!unshare(CLONE_NEWNS | CLONE_NEWPID) &&
              !mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL));
        pid_t nested = fork();
        CHECK(nested >= 0);
        if (!nested)
        {
            CHECK(getpid() == 1 && !mount("proc", "/proc", "proc", 0, NULL));
            CHECK(!stat("/proc/self/ns/pid", &after_pid) &&
                  !stat("/proc/self/ns/mnt", &after_mnt) &&
                  parent_pid.st_ino != after_pid.st_ino &&
                  parent_mnt.st_ino != after_mnt.st_ino);
            CHECK(!mkdir("/tmp/private-view", 0700) &&
                  !mount("tmpfs", "/tmp/private-view", "tmpfs", 0,
                         "size=1048576"));
            int marker = open("/tmp/private-view/owned",
                              O_CREAT | O_EXCL | O_WRONLY, 0600);
            CHECK(marker >= 0 && write(marker, "view", 4) == 4 &&
                  !close(marker));
            printf("OF_NAMESPACE pid=1 parent_pid_ino=%lu child_pid_ino=%lu "
                   "parent_mnt_ino=%lu child_mnt_ino=%lu private_mount=1\n",
                   (unsigned long)parent_pid.st_ino,
                   (unsigned long)after_pid.st_ino,
                   (unsigned long)parent_mnt.st_ino,
                   (unsigned long)after_mnt.st_ino);
            _exit(0);
        }
        int status;
        CHECK(waitpid(nested, &status, 0) == nested && WIFEXITED(status) &&
              !WEXITSTATUS(status));
        _exit(0);
    }
    wait_for(child, 5);
    CHECK(access("/tmp/private-view/owned", F_OK) == -1 && errno == ENOENT);
    puts("OF_NAMESPACE_PARENT private_file_absent=1 budget_unchanged=1");
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    char marker[32] = {0};
    int owned = open("/run/cmastery-owned-os", O_RDONLY);
    CHECK(owned >= 0 && read(owned, marker, sizeof marker - 1) > 0 &&
          !close(owned) && !strcmp(marker, "CM-OS-OWNED-1\n"));
    show("/proc/self", "cgroup");
    namespace_probe();
    CHECK(!mkdir(base, 0700));
    put("/sys/fs/cgroup", "cgroup.subtree_control", "+cpu +memory +io");
    put(base, "cgroup.subtree_control", "+cpu +memory +io");
    show(base, "cgroup.controllers");
    puts("OF_TRIAL_BEGIN ungrouped");
    pid_t baseline = launch(NULL, 1, NULL, 0, 0, 0);
    wait_for(baseline, 8);
    puts("OF_TRIAL_END ungrouped");
    trial("unlimited", "max 100000", "max", 1);
    trial("cpu", "20000 100000", "max", 1);
    trial("memory", "max 100000", "16777216", 1);
    trial("allocation-refused", "max 100000", "max", 3);
    char parent[256], a[256], b[256];
    snprintf(parent, sizeof parent, "%s/weighted", base);
    snprintf(a, sizeof a, "%s/weighted/a", base);
    snprintf(b, sizeof b, "%s/weighted/b", base);
    CHECK(!mkdir(parent, 0700));
    put(parent, "cpu.max", "30000 100000");
    put(parent, "cgroup.subtree_control", "+cpu +memory +io");
    CHECK(!mkdir(a, 0700) && !mkdir(b, 0700));
    put(a, "cpu.weight", "100");
    put(b, "cpu.weight", "400");
    puts("OF_TRIAL_BEGIN weighted");
    snapshot(parent);
    snapshot(a);
    snapshot(b);
    pid_t ca = launch(a, 0, NULL, 0, 0, 0), cb = launch(b, 0, NULL, 0, 0, 0);
    wait_for(ca, 8);
    wait_for(cb, 8);
    snapshot(a);
    snapshot(b);
    snapshot(parent);
    CHECK(!rmdir(a) && !rmdir(b) && !rmdir(parent));
    puts("OF_TRIAL_END weighted cleanup=1");
    snprintf(parent, sizeof parent, "%s/ancestry", base);
    snprintf(a, sizeof a, "%s/ancestry/child", base);
    CHECK(!mkdir(parent, 0700));
    put(parent, "cgroup.subtree_control", "+cpu +memory +io");
    CHECK(!mkdir(a, 0700));
    put(a, "cpu.max", "20000 100000");
    for (unsigned i = 0; i < 2; i++)
    {
        put(parent, "cpu.max", i ? "10000 100000" : "max 100000");
        printf("OF_TRIAL_BEGIN ancestry%u\n", i);
        snapshot(parent);
        snapshot(a);
        ca = launch(a, 0, NULL, 0, 0, 0);
        wait_for(ca, 8);
        snapshot(a);
        snapshot(parent);
        printf("OF_TRIAL_END ancestry%u\n", i);
    }
    CHECK(!rmdir(a) && !rmdir(parent));
    snprintf(a, sizeof a, "%s/io", base);
    CHECK(!mkdir(a, 0700));
    struct stat device;
    CHECK(!stat("/dev/vda", &device) && S_ISBLK(device.st_mode));
    char limit[128];
    snprintf(limit, sizeof limit, "%u:%u wbps=2097152 rbps=2097152",
             major(device.st_rdev), minor(device.st_rdev));
    put(a, "io.max", limit);
    puts("OF_TRIAL_BEGIN io-limit");
    snapshot(a);
    ca = launch(a, 2, "/disk", 0, 1, 0);
    wait_for(ca, 15);
    snapshot(a);
    CHECK(!rmdir(a));
    puts("OF_TRIAL_END io-limit cleanup=1");
    for (unsigned trial_no = 0; trial_no < 3; trial_no++)
    {
        for (unsigned sync = 0; sync < 2; sync++)
        {
            unsigned phase = (sync + trial_no) % 2;
            printf("OF_IO_PAIR trial=%u position=%u sync=%u backend=ext4\n",
                   trial_no, sync, phase);
            io_work("/disk", phase, 0, 0);
        }
    }
    puts("OF_IO_BACKEND_CHANGED tmpfs");
    io_work("/tmp", 0, 1, 0);
    CHECK(!mkdir("/tiny", 0700) &&
          !mount("tmpfs", "/tiny", "tmpfs", 0, "size=2097152"));
    io_work("/tiny", 0, 0, 1);
    CHECK(!umount("/tiny") && !rmdir("/tiny"));
    CHECK(!rmdir(base));
    puts("OF_PASS:guest:namespace:cpu-memory-weight-ancestry-io:cleanup");
    return 0;
}
