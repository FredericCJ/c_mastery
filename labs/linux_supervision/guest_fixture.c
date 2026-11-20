/* Linux x86-64 guest fixture. Only the disposable guest's init invokes this. */
#define _GNU_SOURCE
#include "supervision.h"
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <inttypes.h>
#include <net/if.h>
#include <poll.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define CGROUP "/sys/fs/cgroup"
#define MARKER "/run/supervision-marker"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
enum { UNAVAILABLE = 77, FIXTURE_ERROR = 125 };

static void fail(const char *action)
{
    perror(action);
    exit(FIXTURE_ERROR);
}

static void path_for(char *dest, size_t size, const char *base, const char *file)
{
    int n = snprintf(dest, size, "%s/%s", base, file);
    if (n < 0 || (size_t)n >= size) {
        errno = ENAMETOOLONG;
        fail("fixture path");
    }
}

static void put(const char *dir, const char *name, const char *value, bool create)
{
    char path[256];
    path_for(path, sizeof(path), dir, name);
    int fd = open(path, O_WRONLY | O_CLOEXEC | (create ? O_CREAT | O_EXCL : 0), 0644);
    size_t size = strlen(value);
    if (fd < 0 || write(fd, value, size) != (ssize_t)size || close(fd) < 0)
        fail(path);
}

static void get(const char *dir, const char *name, char *value, size_t size)
{
    char path[256];
    path_for(path, sizeof(path), dir, name);
    FILE *file = fopen(path, "r");
    if (!file || !fgets(value, (int)size, file) || fclose(file) != 0)
        fail(path);
    value[strcspn(value, "\n")] = '\0';
}

static unsigned long long counter(const char *dir, const char *name, const char *key)
{
    char path[256], field[64];
    unsigned long long value;
    path_for(path, sizeof(path), dir, name);
    FILE *file = fopen(path, "r");
    if (!file)
        fail(path);
    while (fscanf(file, "%63s %llu", field, &value) == 2) {
        if (strcmp(field, key) == 0) {
            fclose(file);
            return value;
        }
    }
    fprintf(stderr, "fixture missing %s in %s\n", key, path);
    exit(FIXTURE_ERROR);
}

static int no_members(const char *dir)
{
    char path[256];
    path_for(path, sizeof(path), dir, "cgroup.procs");
    FILE *file = fopen(path, "r");
    if (!file)
        fail(path);
    int empty = fgetc(file) == EOF;
    if (ferror(file) || fclose(file) != 0)
        fail(path);
    return empty;
}

static uintmax_t inode(const char *base, const char *member)
{
    char path[256];
    struct stat st;
    path_for(path, sizeof(path), base, member);
    if (stat(path, &st) != 0)
        fail(path);
    return (uintmax_t)st.st_ino;
}

static void membership(const char *proc, char *buf, size_t size)
{
    char line[256];
    get(proc, "cgroup", line, sizeof(line));
    if (strncmp(line, "0::", 3) != 0 || strlen(line + 3) >= size)
        fail("unexpected cgroup v2 membership");
    strcpy(buf, line + 3);
}

struct trial {
    const char *name;
    const char *variant;
    char group[128];
    char leaf[192];
    char proc[64];
    pid_t pid;
    int channel;
    bool network;
};

static void policy(const struct trial *trial, char pids[32], char quota[32], int *period)
{
    char line[128];
    get(trial->leaf, "pids.max", pids, 32);
    get(trial->leaf, "cpu.max", line, sizeof(line));
    if (sscanf(line, "%31s %d", quota, period) != 2)
        fail("cpu.max");
}

static void loopback_up(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    struct ifreq lo = {.ifr_name = "lo"};
    if (fd < 0 || ioctl(fd, SIOCGIFFLAGS, &lo) < 0)
        fail("get loopback flags");
    lo.ifr_flags |= IFF_UP;
    if (ioctl(fd, SIOCSIFFLAGS, &lo) < 0 || close(fd) < 0)
        fail("enable private loopback");
}

static bool exchange(int fd, char expected)
{
    char byte;
    ssize_t n;
    do {
        n = recv(fd, &byte, 1, 0);
    } while (n < 0 && errno == EINTR);
    return n == 1 && byte == expected;
}

static int inner(void *context)
{
    struct trial *trial = context;
    /* The parent attaches this blocked child before the first workload instruction. */
    if (!exchange(trial->channel, 'S'))
        return FIXTURE_ERROR;
    if (dup2(trial->channel, 3) < 0 || fcntl(3, F_SETFD, 0) != 0 ||
        close_range(4, ~0U, 0) != 0)
        fail("prepare fixture channel");
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) ||
        mount("proc", "/proc", "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC, NULL) ||
        mount("tmpfs", "/run", "tmpfs", MS_NOSUID | MS_NODEV, "size=1m,mode=0755") ||
        mount(CGROUP, CGROUP, NULL, MS_BIND, NULL) ||
        mount(NULL, CGROUP, NULL, MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID |
              MS_NODEV | MS_NOEXEC, NULL))
        fail("prepare private mounts");
    put("/run", "supervision-marker", "inner\n", true);
    put("/run", "linux-supervision-guest", "owned\n", true);
    if (trial->network)
        loopback_up();
    if (setgroups(0, NULL) || setresgid(1000, 1000, 1000) ||
        setresuid(1000, 1000, 1000) || prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
        fail("drop guest fixture privileges");
    char marker[32], group[128];
    get("/run", "supervision-marker", marker, sizeof(marker));
    membership("/proc/self", group, sizeof(group));
    printf("LS_INNER_FIX name=%s variant=%s pid=%d uid=%d gid=%d "
           "pid_ns=%ju mnt_ns=%ju net_ns=%ju marker=%s group=%s\n",
           trial->name, trial->variant, getpid(), getuid(), getgid(),
           inode("/proc/self/ns", "pid"), inode("/proc/self/ns", "mnt"),
           inode("/proc/self/ns", "net"), marker, group);
    /* Hold the live namespace until its external observer has read /proc/PID. */
    if (send(3, "I", 1, MSG_NOSIGNAL) != 1 || !exchange(3, 'E'))
        return FIXTURE_ERROR;
    if (!trial->network && close(3) != 0)
        fail("close fixture channel");
    if (strcmp(trial->name, "smoke") == 0)
        return 0;
    execl("/supervise", "/supervise", trial->name, trial->variant, (char *)NULL);
    fail("exec /supervise");
    return FIXTURE_ERROR;
}

static bool ready(int fd, char expected)
{
    struct pollfd pollfd = {.fd = fd, .events = POLLIN};
    return poll(&pollfd, 1, 12000) == 1 && exchange(fd, expected);
}

static void launch(struct trial *trial, const char *name, const char *variant)
{
    static _Alignas(max_align_t) char stack[128 * 1024];
    trial->name = name;
    trial->variant = variant;
    trial->network = strcmp(name, "network") == 0 || strcmp(name, "smoke") == 0 ||
                     strcmp(name, "telemetry") == 0;
    snprintf(trial->group, sizeof(trial->group), "/lab/%s-%s", name, variant);
    snprintf(trial->leaf, sizeof(trial->leaf), CGROUP "%s", trial->group);
    if (mkdir(trial->leaf, 0755) != 0)
        fail(trial->leaf);
    const char *limit = "24";
    if (strcmp(name, "pids") == 0)
        limit = strcmp(variant, "two") == 0 ? "2" : "3";
    put(trial->leaf, "pids.max", limit, false);
    put(trial->leaf, "cpu.max", strcmp(variant, "quota") == 0 ? "20000 100000" :
        "max 100000", false);

    int channels[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, channels) != 0)
        fail("fixture socketpair");
    trial->channel = channels[1];
    int flags = CLONE_NEWPID | CLONE_NEWNS | SIGCHLD;
    if (trial->network)
        flags |= CLONE_NEWNET;
    trial->pid = clone(inner, stack + sizeof(stack), flags, trial);
    if (trial->pid < 0) {
        int error = errno;
        perror("guest clone namespaces");
        exit(error == EPERM || error == EINVAL || error == ENOSYS ? UNAVAILABLE : FIXTURE_ERROR);
    }
    close(channels[1]);
    trial->channel = channels[0];
    char id[32];
    snprintf(id, sizeof(id), "%d", trial->pid);
    snprintf(trial->proc, sizeof(trial->proc), "/proc/%s", id);
    put(trial->leaf, "cgroup.procs", id, false);
    if (send(trial->channel, "S", 1, MSG_NOSIGNAL) != 1 || !ready(trial->channel, 'I'))
        fail("private guest setup");
    char parent_group[128], child_group[128], marker[32], current[32], pids[32], quota[32];
    int period;
    membership("/proc/self", parent_group, sizeof(parent_group));
    membership(trial->proc, child_group, sizeof(child_group));
    policy(trial, pids, quota, &period);
    get(trial->leaf, "pids.current", current, sizeof(current));
    get("/run", "supervision-marker", marker, sizeof(marker));
    printf("LS_OUTER name=%s variant=%s observer_pid=%d child_pid=%d observer_group=%s "
           "child_group=%s pids_current=%s pids_max=%s cpu_quota=%s cpu_period=%d "
           "pid_ns=%ju mnt_ns=%ju net_ns=%ju marker=%s ",
           name, variant, getpid(), trial->pid, parent_group, child_group, current, pids,
           quota, period, inode("/proc/self/ns", "pid"), inode("/proc/self/ns", "mnt"),
           inode("/proc/self/ns", "net"), marker);
    printf("child_pid_ns=%ju child_mnt_ns=%ju child_net_ns=%ju\n",
           inode(trial->proc, "ns/pid"), inode(trial->proc, "ns/mnt"),
           inode(trial->proc, "ns/net"));
    if (send(trial->channel, "E", 1, MSG_NOSIGNAL) != 1)
        fail("dispatch guest program");
}

static long long monotonic_ms(void)
{
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0)
        fail("clock_gettime");
    return (long long)time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

static int finish(struct trial *trial)
{
    int status;
    long long end = monotonic_ms() + 45000;
    bool killed = false;
    for (;;) {
        pid_t waited = waitpid(trial->pid, &status, WNOHANG);
        if (waited == trial->pid)
            break;
        if (waited < 0 && errno != EINTR)
            fail("waitpid");
        if (!killed && monotonic_ms() > end) {
            fprintf(stderr, "LS_TIMEOUT name=%s variant=%s\n", trial->name, trial->variant);
            if (kill(trial->pid, SIGKILL) < 0 && errno != ESRCH)
                fail("kill overdue guest child");
            killed = true;
        }
        usleep(10000);
    }
    close(trial->channel);
    char current[32], parent_group[128], marker[32], pids[32], quota[32];
    int period;
    membership("/proc/self", parent_group, sizeof(parent_group));
    get("/run", "supervision-marker", marker, sizeof(marker));
    policy(trial, pids, quota, &period);
    end = monotonic_ms() + 2000;
    do {
        get(trial->leaf, "pids.current", current, sizeof(current));
        if (strcmp(current, "0") == 0)
            break;
        usleep(10000);
    } while (monotonic_ms() < end);
    unsigned long long hits = counter(trial->leaf, "pids.events", "max");
    unsigned long long throttles = counter(trial->leaf, "cpu.stat", "nr_throttled");
    unsigned long long usec = counter(trial->leaf, "cpu.stat", "usage_usec");
    int empty = no_members(trial->leaf);
    if (rmdir(trial->leaf) != 0)
        fail(trial->leaf);
    printf("LS_CLEAN name=%s variant=%s pids_current=%s pids_hits=%llu nr_throttled=%llu "
           "usage_usec=%llu removed=1 marker=%s observer_group=%s child_group=%s "
           "pids_max=%s cpu_quota=%s cpu_period=%d child_pid=%d empty_procs=%d\n",
           trial->name, trial->variant, current, hits, throttles, usec, marker, parent_group,
           trial->group, pids, quota, period, trial->pid, empty);
    int result = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
    if (strcmp(trial->name, "smoke") != 0)
        printf("LS_CASE_END name=%s variant=%s status=%d\n", trial->name, trial->variant, result);
    return result;
}

static unsigned long long tcp_inode(const struct trial *trial, unsigned wanted_state,
                                    unsigned *client_port)
{
    char path[256], line[512];
    path_for(path, sizeof(path), trial->proc, "net/tcp");
    FILE *file = fopen(path, "r");
    if (!file)
        fail(path);
    unsigned long long socket_id = 0;
    while (fgets(line, sizeof(line), file)) {
        unsigned addr, port, remote, remote_port, state;
        unsigned long long candidate;
        int fields = sscanf(line, " %*u: %x:%x %x:%x %x %*s %*s %*s %*s %*s %llu",
                            &addr, &port, &remote, &remote_port, &state, &candidate);
        /* The pinned little-endian guest exposes reversed IPv4 hex octets in
         * /proc/net/tcp, followed by the TCP state and decimal inode. */
        if (fields == 6 && addr == 0x0100007f && port == SUPERVISION_PORT &&
            state == wanted_state && (!client_port || (remote == addr && remote_port > 0))) {
            socket_id = candidate;
            if (client_port) *client_port = remote_port;
            break;
        }
    }
    if (ferror(file) || fclose(file) != 0)
        fail(path);
    return socket_id;
}

static bool observe_telemetry(struct trial *trial)
{
    unsigned client_port = 0;
    if (ready(trial->channel, 'R')) {
        unsigned long long inode = tcp_inode(trial, 0x01, &client_port);
        if (inode) {
            unsigned long long usage = counter(trial->leaf, "cpu.stat", "usage_usec");
            unsigned long long throttled = counter(trial->leaf, "cpu.stat", "nr_throttled");
            printf("LS_TCP name=telemetry variant=%s port=%d client_port=%u "
                   "server_inode=%llu usage_usec=%llu nr_throttled=%llu\n",
                   trial->variant, SUPERVISION_PORT, client_port, inode, usage, throttled);
            if (send(trial->channel, "G", 1, MSG_NOSIGNAL) == 1 && ready(trial->channel, 'D')) {
                usage = counter(trial->leaf, "cpu.stat", "usage_usec");
                throttled = counter(trial->leaf, "cpu.stat", "nr_throttled");
                printf("LS_CPU_END name=telemetry variant=%s usage_usec=%llu nr_throttled=%llu\n",
                       trial->variant, usage, throttled);
                return send(trial->channel, "F", 1, MSG_NOSIGNAL) == 1;
            }
        }
    }
    puts("LS_CONTROL_FAIL name=telemetry reason=missing-established-loopback");
    shutdown(trial->channel, SHUT_RDWR);
    return false;
}

static int run_network(void)
{
    struct trial pair[2];
    launch(&pair[0], "network", "A");
    launch(&pair[1], "network", "B");
    bool overlap = ready(pair[0].channel, 'R') && ready(pair[1].channel, 'R');
    if (overlap) {
        unsigned long long a_socket = tcp_inode(&pair[0], 0x0a, NULL);
        unsigned long long b_socket = tcp_inode(&pair[1], 0x0a, NULL);
        overlap = a_socket != 0 && b_socket != 0 && a_socket != b_socket;
        if (overlap)
            printf("LS_NET_OVERLAP a_pid=%d b_pid=%d ready=2 a_socket=%llu b_socket=%llu "
                   "port=%d\n", pair[0].pid, pair[1].pid, a_socket, b_socket, SUPERVISION_PORT);
    }
    if (overlap) {
        for (size_t i = 0; i < ARRAY_SIZE(pair); ++i) {
            if (send(pair[i].channel, "G", 1, MSG_NOSIGNAL) != 1)
                overlap = false;
        }
    }
    if (!overlap) {
        puts("LS_CONTROL_FAIL name=network reason=missing-live-listener-pair");
        /* EOF also releases the peer if the other listener never becomes ready. */
        shutdown(pair[0].channel, SHUT_RDWR);
        shutdown(pair[1].channel, SHUT_RDWR);
    }
    int a = finish(&pair[0]);
    int b = finish(&pair[1]);
    return !overlap || a != 0 || b != 0;
}

int main(int argc, char **argv)
{
    const struct {
        const char *name;
        const char *variants[5];
    } cases[] = {
        {"smoke", {"probe"}}, {"namespace", {"view"}}, {"pids", {"two", "three"}},
        {"shutdown", {"normal", "early", "startfail", "stubborn"}},
        {"telemetry", {"baseline", "quota", "stalled"}},
    };
    setvbuf(stdout, NULL, _IONBF, 0);
    char marker[16];
    get("/run", "supervision-marker", marker, sizeof(marker));
    if (argc != 2 || getppid() != 1 || geteuid() != 0 || strcmp(marker, "outer") != 0)
        return FIXTURE_ERROR;
    if (strcmp(argv[1], "network") == 0)
        return run_network();
    for (size_t i = 0; i < ARRAY_SIZE(cases); ++i) {
        if (strcmp(argv[1], cases[i].name) != 0)
            continue;
        int result = 0;
        for (size_t j = 0; cases[i].variants[j]; ++j) {
            struct trial trial;
            launch(&trial, cases[i].name, cases[i].variants[j]);
            if (strcmp(trial.name, "telemetry") == 0 && !observe_telemetry(&trial))
                result = 1;
            if (finish(&trial) != 0)
                result = 1;
        }
        return result;
    }
    return FIXTURE_ERROR;
}
