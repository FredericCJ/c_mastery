#define _POSIX_C_SOURCE 200809L
#include "supervision.h"
#include "support.h"

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* All waiting siblings inherit the read end. Closing the last writer releases
 * every admitted task, even when a fork attempt returned EAGAIN. */
static pid_t held_worker(const int release[2], int64_t deadline)
{
    pid_t child = fork();
    if (child == 0) {
        close(release[1]);
        char unexpected;
        await_fd(release[0], POLLIN, deadline);
        if (read(release[0], &unexpected, 1) != 0) _exit(1);
        close(release[0]);
        _exit(0);
    }
    return child;
}

int supervise_pids(const char *variant)
{
    if (strcmp(variant, "two") && strcmp(variant, "three")) return 2;
    int release[2];
    int64_t deadline = now_ms() + 5000;
    if (pipe(release)) fail_sys("pids release pipe");
    char leaf[256], limit[32];
    leaf_membership(leaf, sizeof leaf);
    leaf_field(leaf, "pids.max", limit, sizeof limit);
    unsigned long long before = leaf_counter(leaf, "pids.current", NULL);
    pid_t first = held_worker(release, deadline);
    if (first < 0) fail_sys("first fork");
    unsigned long long occupied = leaf_counter(leaf, "pids.current", NULL);
    unsigned long long event_before = leaf_counter(leaf, "pids.events", "max");
    pid_t next = held_worker(release, deadline);
    int fork_errno = next < 0 ? errno : 0;
    unsigned long long event_after = leaf_counter(leaf, "pids.events", "max");
    unsigned long long peak = leaf_counter(leaf, "pids.current", NULL);
    close(release[0]);
    close(release[1]);
    pid_t children[] = {first, next};
    int reaped = 0;
    for (size_t i = 0; i < sizeof children / sizeof children[0]; ++i) {
        if (children[i] < 0) continue;
        int status = collect(children[i], deadline);
        if (!WIFEXITED(status) || WEXITSTATUS(status)) fail_protocol("worker failed");
        ++reaped;
    }
    unsigned long long remaining = leaf_counter(leaf, "pids.current", NULL);
    printf("LS pids variant=%s cgroup=%s limit=%s before=%llu occupied=%llu "
           "attempted_pid=%ld fork_errno=%d peak=%llu event_before=%llu "
           "event_after=%llu remaining=%llu reaped=%d\n", variant, leaf, limit,
           before, occupied, (long)next, fork_errno, peak, event_before,
           event_after, remaining, reaped);
    return 0;
}
