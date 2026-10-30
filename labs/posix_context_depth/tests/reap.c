#define _GNU_SOURCE
#include "context.h"
#include "test.h"
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

enum
{
    TICKET_OWNED = 1,
    TICKET_COLLECTED = 2,
    TICKET_AUTHORITY_LOST = 3,
    KIND_EXITED = 1,
    KIND_SIGNALED = 3,
    /* child() result meaning "terminate yourself with SIGTERM". */
    CHILD_SIGNAL_ITSELF = -1,
    /* Exit statuses a child uses to report its own setup failure. */
    CHILD_GATE_FAILED = 90,
    CHILD_KILL_FAILED = 91,
    CHILD_SURVIVED_KILL = 92
};

/* Fault-injecting wait hook: fails once with `error`, then forwards to the
 * real waitpid(). `calls` proves how many waits a ticket really issued. */
struct pc_fault
{
    unsigned calls;
    int error;
};

static pid_t once(pid_t pid, int *status, void *context)
{
    struct pc_fault *fault = context;
    ++fault->calls;
    if (fault->error)
    {
        int error = fault->error;
        fault->error = 0;
        errno = error;
        return -1;
    }
    return waitpid(pid, status, 0);
}

/* Forks a child that blocks reading `gate` until the parent releases it, so
 * the parent decides when the child becomes collectable. The child exits with
 * `result`, or terminates itself with SIGTERM for CHILD_SIGNAL_ITSELF. The
 * parent keeps only gate[1]; release() closes it. */
static pid_t child(int gate[2], int result)
{
    CHECK(pipe(gate) == 0);
    pid_t pid = fork();
    CHECK(pid >= 0);
    if (!pid)
    {
        char token;
        close(gate[1]);
        ssize_t got;
        do
        {
            got = read(gate[0], &token, 1);
        } while (got < 0 && errno == EINTR);
        if (got != 1)
        {
            _exit(CHILD_GATE_FAILED);
        }
        close(gate[0]);
        if (result == CHILD_SIGNAL_ITSELF)
        {
            if (kill(getpid(), SIGTERM))
            {
                _exit(CHILD_KILL_FAILED);
            }
            _exit(CHILD_SURVIVED_KILL);
        }
        _exit(result);
    }
    close(gate[0]);
    return pid;
}

static void release(int gate[2])
{
    CHECK(write(gate[1], "x", 1) == 1 && close(gate[1]) == 0);
}

/* A failed wait must leave the ticket owned and the result untouched; the
 * retry then collects exactly once, and a collected ticket never waits again. */
static void deferred_wait_error(int injected_error)
{
    int gate[2];
    pid_t pid = child(gate, 127);
    struct pc_ticket ticket = {pid, TICKET_OWNED};
    struct pc_launch out = {99, 98, 97};
    struct pc_launch untouched = out;
    struct pc_fault fault = {0, injected_error};
    CHECK(pc_reap(&ticket, once, &fault, &out) == injected_error);
    CHECK(ticket.phase == TICKET_OWNED && ticket.pid == pid &&
          !memcmp(&out, &untouched, sizeof out));
    release(gate);
    CHECK(pc_reap(&ticket, once, &fault, &out) == 0 &&
          ticket.phase == TICKET_COLLECTED && ticket.pid == 0 &&
          out.kind == KIND_EXITED && out.value == 127 && out.pid == pid);
    unsigned calls = fault.calls;
    CHECK(pc_reap(&ticket, once, &fault, &out) == EALREADY &&
          fault.calls == calls);
    printf(
        "PC_REAP deferred_error=%d retained_then_collected=1 no_duplicate_wait=1\n",
        injected_error);
}

/* EINTR on the first wait is retried inside pc_reap (two calls in total). */
static void interrupted_wait(int child_result)
{
    int gate[2];
    pid_t pid = child(gate, child_result);
    struct pc_ticket ticket = {pid, TICKET_OWNED};
    struct pc_launch out = {0};
    struct pc_fault fault = {0, EINTR};
    release(gate);
    CHECK(pc_reap(&ticket, once, &fault, &out) == 0 && fault.calls == 2 &&
          out.pid == pid);
    CHECK(child_result == 0
              ? (out.kind == KIND_EXITED && out.value == 0)
              : (out.kind == KIND_SIGNALED && out.value == SIGTERM));
    printf("PC_REAP interrupted_then_collected=1 kind=%d value=%d\n", out.kind,
           out.value);
}

/* A competing reaper violates the exclusive-owner assumption: the ticket's
 * wait fails with a real ECHILD and the authority is retired for good. */
static void competing_reaper(void)
{
    int gate[2];
    int status;
    pid_t pid = child(gate, 0);
    struct pc_ticket ticket = {pid, TICKET_OWNED};
    release(gate);
    CHECK(waitpid(pid, &status, 0) == pid);
    struct pc_launch out = {99, 98, 97};
    struct pc_launch untouched = out;
    struct pc_fault fault = {0, 0};
    CHECK(pc_reap(&ticket, once, &fault, &out) == ECHILD &&
          ticket.phase == TICKET_AUTHORITY_LOST && ticket.pid == 0 &&
          !memcmp(&out, &untouched, sizeof out));
    CHECK(pc_reap(&ticket, once, &fault, &out) == EALREADY && fault.calls == 1);
    puts("PC_REAP actual_ECHILD retired_authority no_signal_no_second_wait");
}

int main(void)
{
    deferred_wait_error(EIO);
    deferred_wait_error(EINVAL);
    interrupted_wait(0);
    interrupted_wait(CHILD_SIGNAL_ITSELF);
    competing_reaper();
    puts("PC_REAP_PASS");
    return 0;
}
