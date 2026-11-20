#define _POSIX_C_SOURCE 200809L
#include "supervision.h"
#include "support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int supervise_namespace(const char *variant)
{
    if (strcmp(variant, "view")) return 2;
    int report[2];
    if (pipe(report)) fail_sys("namespace report pipe");
    int64_t deadline = now_ms() + 3000;
    pid_t worker = fork();
    if (worker < 0) fail_sys("namespace fork");
    if (worker == 0) {
        close(report[0]);
        pid_t identity[2] = {getpid(), getppid()};
        write_full(report[1], identity, sizeof identity, deadline);
        _exit(0);
    }
    close(report[1]);
    pid_t identity[2];
    read_full(report[0], identity, sizeof identity, deadline);
    close(report[0]);
    int status = collect(worker, deadline);
    if (!WIFEXITED(status) || WEXITSTATUS(status) || worker != identity[0] ||
        getpid() != identity[1]) fail_protocol("worker identity mismatch");

    char marker[32], leaf[256], limit[32], self[32];
    ssize_t length = readlink("/proc/self", self, sizeof self - 1);
    if (length < 0) fail_sys("readlink /proc/self");
    self[length] = '\0';
    read_field("/run/supervision-marker", marker, sizeof marker);
    leaf_membership(leaf, sizeof leaf);
    leaf_field(leaf, "pids.max", limit, sizeof limit);
    printf("LS view variant=%s inner_pid=%ld worker_pid=%ld proc_pid=%s "
           "report_pid=%ld report_ppid=%ld pid_ns=%llu mount_ns=%llu marker=%s cgroup=%s pids_max=%s\n",
           variant, (long)getpid(), (long)worker, self, (long)identity[0],
           (long)identity[1], namespace_inode("pid"),
           namespace_inode("mnt"), marker, leaf, limit);
    return 0;
}
