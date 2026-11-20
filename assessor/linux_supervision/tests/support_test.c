#define _POSIX_C_SOURCE 200809L
#include "../support.h"

#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* Run only host-safe pipe and reaping paths here. The full cases run in QEMU. */
static int rejected_read(int hold_writer)
{
    int fds[2];
    if (pipe(fds)) fail_sys("test pipe");
    pid_t child = fork();
    if (child < 0) fail_sys("test fork");
    if (!child) {
        close(fds[1]);
        char message[2];
        read_full(fds[0], message, sizeof message, now_ms() + 100);
        _exit(0);
    }
    close(fds[0]);
    if (!hold_writer) {
        write_full(fds[1], "a", 1, now_ms() + 1000);
        close(fds[1]);
    }
    int status = collect(child, now_ms() + 3000);
    if (hold_writer) close(fds[1]);
    return WIFEXITED(status) && WEXITSTATUS(status) == 1;
}

int main(void)
{
    int initial = count_fds();
    int fds[2];
    if (pipe(fds)) fail_sys("test pipe");
    if (count_fds() != initial + 2) return 1;
    char message[2];
    int64_t deadline = now_ms() + 1000;
    write_full(fds[1], "a", 1, deadline);
    write_full(fds[1], "b", 1, deadline);
    close(fds[1]);
    read_full(fds[0], message, sizeof message, deadline);
    await_fd(fds[0], POLLIN, deadline);
    char extra;
    if (memcmp(message, "ab", 2) || read(fds[0], &extra, 1) != 0) return 1;
    close(fds[0]);
    if (!rejected_read(0) || !rejected_read(1) || count_fds() != initial) return 1;
    puts("private host pipe checks passed");
    return 0;
}
