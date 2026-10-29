#define _GNU_SOURCE
#include "context.h"
#include "test.h"
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct pc_fault
{
    unsigned calls;
    int error;
};

static pid_t once(pid_t p, int *status, void *v)
{
    struct pc_fault *f = v;
    ++f->calls;
    if (f->error)
    {
        int e = f->error;
        f->error = 0;
        errno = e;
        return -1;
    }
    return waitpid(p, status, 0);
}

static pid_t child(int gate[2], int result)
{
    CHECK(pipe(gate) == 0);
    pid_t p = fork();
    CHECK(p >= 0);
    if (!p)
    {
        char c;
        close(gate[1]);
        ssize_t n;
        do
        {
            n = read(gate[0], &c, 1);
        } while (n < 0 && errno == EINTR);
        if (n != 1)
            _exit(90);
        close(gate[0]);
        if (result < 0)
        {
            if (kill(getpid(), SIGTERM))
                _exit(91);
            _exit(92);
        }
        _exit(result);
    }
    close(gate[0]);
    return p;
}

static void release(int gate[2])
{
    CHECK(write(gate[1], "x", 1) == 1 && close(gate[1]) == 0);
}

int main(void)
{
    for (int error_case = 0; error_case < 2; ++error_case)
    {
        int gate[2];
        pid_t pid = child(gate, 127);
        struct pc_ticket t = {pid, 1};
        struct pc_launch out = {99, 98, 97}, old = out;
        struct pc_fault f = {0, error_case ? EINVAL : EIO};
        CHECK(pc_reap(&t, once, &f, &out) == (error_case ? EINVAL : EIO));
        CHECK(t.phase == 1 && t.pid == pid && !memcmp(&out, &old, sizeof out));
        release(gate);
        CHECK(pc_reap(&t, once, &f, &out) == 0 && t.phase == 2 && t.pid == 0 &&
              out.kind == 1 && out.value == 127 && out.pid == pid);
        unsigned calls = f.calls;
        CHECK(pc_reap(&t, once, &f, &out) == EALREADY && f.calls == calls);
        printf(
            "PC_REAP deferred_error=%d retained_then_collected=1 no_duplicate_wait=1\n",
            error_case ? EINVAL : EIO);
    }
    for (int result = 0; result >= -1; --result)
    {
        int gate[2];
        pid_t pid = child(gate, result);
        struct pc_ticket t = {pid, 1};
        struct pc_launch out = {0};
        struct pc_fault f = {0, EINTR};
        release(gate);
        CHECK(pc_reap(&t, once, &f, &out) == 0 && f.calls == 2 &&
              out.pid == pid);
        CHECK(result == 0 ? (out.kind == 1 && out.value == 0)
                          : (out.kind == 3 && out.value == SIGTERM));
        printf("PC_REAP interrupted_then_collected=1 kind=%d value=%d\n",
               out.kind, out.value);
    }
    int gate[2], status;
    pid_t pid = child(gate, 0);
    struct pc_ticket t = {pid, 1};
    release(gate);
    CHECK(
        waitpid(pid, &status, 0) ==
        pid); /* Controlled competing reaper violates exclusive-owner assumption. */
    struct pc_launch out = {99, 98, 97}, old = out;
    struct pc_fault f = {0, 0};
    CHECK(pc_reap(&t, once, &f, &out) == ECHILD && t.phase == 3 && t.pid == 0 &&
          !memcmp(&out, &old, sizeof out));
    CHECK(pc_reap(&t, once, &f, &out) == EALREADY && f.calls == 1);
    puts("PC_REAP actual_ECHILD retired_authority no_signal_no_second_wait");
    puts("PC_REAP_PASS");
    return 0;
}
