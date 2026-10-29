#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#ifndef PF_SKIP_MOVE
#define PF_SKIP_MOVE 0
#endif
#ifndef PF_SKIP_RESTORE
#define PF_SKIP_RESTORE 0
#endif
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "PC_FAIL:%d:%s:errno=%d\n", __LINE__, #x, errno); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)
static const char *root = "/sys/fs/cgroup", *base = "/sys/fs/cgroup/t43",
                  *control = "/sys/fs/cgroup/t43/control",
                  *work = "/sys/fs/cgroup/t43/work";

static void path(char *out, const char *dir, const char *name)
{
    int n = snprintf(out, 512, "%s/%s", dir, name);
    CHECK(n > 0 && n < 512);
}

static void put(const char *dir, const char *name, const char *value)
{
    char p[512];
    path(p, dir, name);
    int fd = open(p, O_WRONLY | O_CLOEXEC);
    CHECK(fd >= 0);
    size_t n = strlen(value);
    CHECK(write(fd, value, n) == (ssize_t)n);
    CHECK(!close(fd));
}

static void get(const char *dir, const char *name, char *out, size_t cap)
{
    char p[512];
    path(p, dir, name);
    int fd = open(p, O_RDONLY | O_CLOEXEC);
    CHECK(fd >= 0);
    ssize_t n = read(fd, out, cap - 1);
    CHECK(n >= 0 && (size_t)n < cap - 1);
    out[n] = 0;
    CHECK(!close(fd));
}

static void raw(const char *dir, const char *name)
{
    char p[512], buf[4096];
    path(p, dir, name);
    int fd = open(p, O_RDONLY | O_CLOEXEC);
    if (fd < 0)
    {
        printf("PC_ABSENT %s errno=%d\n", p, errno);
        return;
    }
    printf("PC_RAW_BEGIN %s\n", p);
    ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0)
    {
        CHECK(fwrite(buf, 1, (size_t)n, stdout) == (size_t)n);
    }
    CHECK(!n);
    CHECK(!close(fd));
    printf("PC_RAW_END %s\n", p);
}

static void snapshot(const char *dir)
{
    const char *names[] = {"cgroup.procs",
                           "cgroup.controllers",
                           "cgroup.subtree_control",
                           "cgroup.type",
                           "cpu.max",
                           "cpu.max.burst",
                           "cpu.stat",
                           "memory.max",
                           "memory.current",
                           "memory.peak",
                           "memory.events",
                           "io.stat",
                           "cpu.pressure",
                           "memory.pressure",
                           "io.pressure"};
    for (size_t i = 0; i < sizeof names / sizeof *names; i++)
    {
        raw(dir, names[i]);
    }
}

static void own(const char *dir, const char *name)
{
    char p[512];
    path(p, dir, name);
    CHECK(!chown(p, 1000, 1000));
}

static void move(const char *dir, pid_t pid)
{
    char n[32];
    CHECK(snprintf(n, sizeof n, "%ld", (long)pid) > 0);
    put(dir, "cgroup.procs", n);
}

static void member(const char *expected)
{
    char s[256];
    get("/proc/self", "cgroup", s, sizeof s);
    CHECK(!strcmp(s, expected));
    printf("PC_MEMBER uid=%ld pid=%ld %s", (long)geteuid(), (long)getpid(), s);
}

static int observe_until_exit(pid_t child)
{
    struct timespec start, now, nap = {0, 2000000};
    CHECK(!clock_gettime(CLOCK_MONOTONIC, &start));
    int status;
    char oldp[512] = "", oldt[512] = "";
    for (;;)
    {
        char procs[512], threads[512];
        get(work, "cgroup.procs", procs, sizeof procs);
        get(work, "cgroup.threads", threads, sizeof threads);
        if (strcmp(procs, oldp))
        {
            strcpy(oldp, procs);
            for (char *p = procs; *p; p++)
            {
                if (*p == '\n')
                {
                    *p = ' ';
                }
            }
            printf("PC_LIVE kind=processes ids=%s\n", procs);
        }
        if (strcmp(threads, oldt))
        {
            strcpy(oldt, threads);
            for (char *p = threads; *p; p++)
            {
                if (*p == '\n')
                {
                    *p = ' ';
                }
            }
            printf("PC_LIVE kind=threads ids=%s\n", threads);
        }
        pid_t done = waitpid(child, &status, WNOHANG);
        CHECK(done >= 0);
        if (done == child)
        {
            return status;
        }
        CHECK(!clock_gettime(CLOCK_MONOTONIC, &now));
        CHECK(now.tv_sec - start.tv_sec < 60);
        CHECK(!nanosleep(&nap, NULL));
    }
}

static void trial(const char *name, const char *quota)
{
    put(work, "cpu.max", quota);
    printf("PC_TRIAL_BEGIN %s\n", name);
    int ready[2], go[2];
    CHECK(!pipe2(ready, O_CLOEXEC));
    CHECK(!pipe2(go, O_CLOEXEC));
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        CHECK(!close(ready[0]));
        CHECK(!close(go[1]));
        if (!PF_SKIP_MOVE)
        {
            move(work, getpid());
        }
        member("0::/t43/work\n");
        CHECK(write(ready[1], "R", 1) == 1);
        CHECK(!close(ready[1]));
        char token;
        CHECK(read(go[0], &token, 1) == 1 && token == 'G');
        CHECK(!close(go[0]));
        execl("/workload", "workload", (char *)NULL);
        CHECK(0);
    }
    CHECK(!close(ready[1]));
    CHECK(!close(go[0]));
    char token;
    CHECK(read(ready[0], &token, 1) == 1 && token == 'R');
    CHECK(!close(ready[0]));
    member("0::/t43/control\n");
    printf("PC_PHASE before\n");
    snapshot(work);
    snapshot(base);
    printf("PC_WORKLOAD_BEGIN\n");
    CHECK(write(go[1], "G", 1) == 1);
    CHECK(!close(go[1]));
    int status = observe_until_exit(child);
    printf("PC_WORKLOAD_END raw=%d exited=%d code=%d signal=%d\n", status,
           WIFEXITED(status), WIFEXITED(status) ? WEXITSTATUS(status) : -1,
           WIFSIGNALED(status) ? WTERMSIG(status) : 0);
    CHECK(WIFEXITED(status) && !WEXITSTATUS(status));
    printf("PC_PHASE after\n");
    snapshot(work);
    snapshot(base);
    printf("PC_TRIAL_END %s\n", name);
}

static void delegated(void)
{
    CHECK(geteuid() == 1000 && getegid() == 1000);
    member("0::/t43/control\n");
    char before[128], after[128];
    get(work, "cpu.max", before, sizeof before);
    CHECK(!strcmp(before, "max 100000\n"));
    errno = 0;
    int fd = open("/sys/fs/cgroup/t43/cpu.max", O_WRONLY | O_CLOEXEC);
    CHECK(fd < 0 && errno == EACCES);
    puts("PC_ANCESTOR_POLICY_WRITE_DENIED");
    errno = 0;
    fd = open("/sys/fs/cgroup/cgroup.procs", O_WRONLY | O_CLOEXEC);
    CHECK(fd < 0 && errno == EACCES);
    puts("PC_OUTSIDE_MIGRATION_ACCESS_DENIED");
    puts("PC_INVENTORY_BEGIN");
    snapshot(work);
    snapshot(control);
    snapshot(base);
    snapshot(root);
    raw("/proc/self", "mountinfo");
    puts("PC_INVENTORY_END");
    trial("unlimited", "max 100000");
    trial("quota", "20000 100000");
    if (!PF_SKIP_RESTORE)
    {
        put(work, "cpu.max", before);
    }
    get(work, "cpu.max", after, sizeof after);
    CHECK(!strcmp(before, after));
    printf("PC_RESTORED %s", after);
    puts("PC_DELEGATE_PASS");
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    char marker[128];
    get("/run", "t43-owned", marker, sizeof marker);
    CHECK(!strcmp(marker, "CM-T43-DISPOSABLE-GUEST-1\n"));
    get("/proc", "cmdline", marker, sizeof marker);
    CHECK(strstr(marker, "t43_owned=1"));
    CHECK(geteuid() == 0);
    member("0::/\n");
    CHECK(!mkdir(base, 0700));
    put(root, "cgroup.subtree_control", "+cpu +memory +io");
    put(base, "cgroup.subtree_control", "+cpu +memory +io");
    CHECK(!mkdir(control, 0700));
    CHECK(!mkdir(work, 0700));
    CHECK(!chown(base, 1000, 1000));
    own(base, "cgroup.procs");
    own(base, "cgroup.threads");
    own(base, "cgroup.subtree_control");
    CHECK(!chown(control, 1000, 1000));
    own(control, "cgroup.procs");
    CHECK(!chown(work, 1000, 1000));
    own(work, "cgroup.procs");
    own(work, "cpu.max");
    puts("PC_PREPARED uid=1000 boundary=/t43 supervisor=/control budget=/work");
    int gate[2];
    CHECK(!pipe2(gate, O_CLOEXEC));
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        CHECK(!close(gate[1]));
        char token;
        CHECK(read(gate[0], &token, 1) == 1 && token == 'G');
        CHECK(!close(gate[0]));
        CHECK(!setgroups(0, NULL));
        CHECK(!setgid(1000));
        CHECK(!setuid(1000));
        delegated();
        return 0;
    }
    CHECK(!close(gate[0]));
    move(control, child);
    CHECK(write(gate[1], "G", 1) == 1);
    CHECK(!close(gate[1]));
    int status;
    CHECK(waitpid(child, &status, 0) == child);
    printf("PC_DELEGATE_EXIT raw=%d\n", status);
    put(work, "cgroup.kill", "1");
    put(work, "cpu.max", "max 100000");
    CHECK(!rmdir(work));
    CHECK(!rmdir(control));
    CHECK(!rmdir(base));
    member("0::/\n");
    puts("PC_CLEANUP removed=3");
    if (!WIFEXITED(status) || WEXITSTATUS(status))
    {
        return 1;
    }
    puts(
        "PC_PASS:T43.11.a-DELAYED:original-workload:delegated-uid1000:quota:restore");
    return 0;
}
