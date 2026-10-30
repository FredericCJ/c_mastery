#define _GNU_SOURCE
#include "posix_lifecycle.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <sys/wait.h>
#include <unistd.h>
#ifndef PL_MUTANT
#define PL_MUTANT 0
#endif

/* Fault-injection stages accepted by pl_launch() as fail_stage. */
enum
{
    PL_STAGE_NONE = 0,
    PL_STAGE_PIPE = 1, /* behave as if descriptor acquisition failed */
    PL_STAGE_FORK = 2 /* behave as if fork() failed */
};

/* Exit status of a child whose execve() failed. 127 is also a status the
   program being launched may legitimately return, so it never classifies a
   result by itself: only the error record on the channel does. */
enum
{
    PL_CHILD_EXEC_FAILED = 127
};

/* pl_pool() runs at most this many threads, so its bookkeeping lives in
   fixed arrays on the caller's stack and needs no allocation. */
enum
{
    PL_MAX_WORKERS = 4
};

/* Exit status used when pthread_join() fails, see join_workers(). */
enum
{
    PL_JOIN_FATAL_STATUS = 125
};

/* Child side of the error channel: send every byte of the errno value, or
   give up. A short record is not repaired here; the parent reports it as a
   protocol error. Only async-signal-safe calls (write, _exit) are used,
   because this runs between fork() and _exit() in a possibly multithreaded
   process. */
static void send_exec_error(int fd, int error)
{
    const unsigned char *bytes = (const unsigned char *)&error;
    size_t sent = 0;
    while (sent < sizeof error)
    {
        ssize_t count = write(fd, bytes + sent, sizeof error - sent);
        if (count > 0)
        {
            sent += (size_t)count;
        }
        else if (count < 0 && errno == EINTR)
        {
            continue;
        }
        else
        {
            break;
        }
    }
}

/* Child side after fork(). The read end is closed because the child never
   reads. The write end is O_CLOEXEC: a successful execve() closes it, so
   the parent sees EOF once every write end is closed. EOF with no bytes
   therefore means only that no exec failure was reported; it does not prove
   that exec happened, since a child killed before execve() also closes the
   pipe. An errno record means execve() failed. The environment is
   deliberately empty. Never returns. */
static _Noreturn void run_child(const char *path, char *const argv[],
                                const int channel[2])
{
    (void)close(channel[0]);
    char *const environment[] = {NULL};
    execve(path, argv, environment);
    /* execve() only returns on failure; errno is still its error. */
    send_exec_error(channel[1], errno);
    _exit(PL_CHILD_EXEC_FAILED);
}

/* Reads the exec-error record from the channel until EOF or the record is
   complete. Returns 0 or the errno of a read failure; *used is the number of
   record bytes received (0 bytes: no exec failure was reported; classify by
   wait status). */
static int read_exec_error(int fd, int *exec_error, size_t *used)
{
    *used = 0;
    while (*used < sizeof *exec_error)
    {
        ssize_t count = read(fd, (unsigned char *)exec_error + *used,
                             sizeof *exec_error - *used);
        if (count > 0)
        {
            *used += (size_t)count;
        }
        else if (count == 0)
        {
            return 0;
        }
        else if (errno != EINTR)
        {
            return errno;
        }
    }
    return 0;
}

/* Maps a wait status to a result. Returns EPROTO for a status that is
   neither exit nor signal death (stop and continue are never requested). */
static int classify_status(int status, struct pl_launch *result)
{
    if (WIFEXITED(status))
    {
        *result = (struct pl_launch){PL_EXITED, WEXITSTATUS(status)};
        return 0;
    }
    if (WIFSIGNALED(status))
    {
        *result = (struct pl_launch){PL_SIGNALED, WTERMSIG(status)};
        return 0;
    }
    return EPROTO;
}

/* Fork, or simulate a fork failure for the fault-injection stage. The
   simulated failure sets errno exactly as the real one would. */
static pid_t fork_or_inject(unsigned fail_stage)
{
    if (fail_stage == PL_STAGE_FORK)
    {
        errno = EAGAIN;
        return -1;
    }
    return fork();
}

/* Contract: see CONTRACTS.md. The parent owns the channel until return and
   every path closes both ends, so no descriptor leaks on any outcome. A
   forked child is reaped before return unless waitpid() fails with an error
   other than EINTR; that error is returned and the child stays unreaped
   (hard wait failure is outside the contract). On failure *out is
   untouched. Blocks until the child terminates. */
int pl_launch(const char *path, char *const argv[], unsigned fail_stage,
              struct pl_launch *out)
{
    if (!path || !argv || !argv[0] || !out || fail_stage > PL_STAGE_FORK)
    {
        return EINVAL;
    }
    if (fail_stage == PL_STAGE_PIPE)
    {
        return EMFILE;
    }
    int channel[2];
    if (pipe2(channel, O_CLOEXEC) < 0)
    {
        return errno;
    }
    pid_t child = fork_or_inject(fail_stage);
    if (child < 0)
    {
        int error = errno;
        (void)close(channel[0]);
        (void)close(channel[1]);
        return error;
    }
    if (child == 0)
    {
        run_child(path, argv, channel);
    }
    /* Parent: close the write end first, otherwise the read below never sees
       EOF when exec succeeds. */
    (void)close(channel[1]);
    int exec_error = 0;
    size_t used = 0;
    int read_error = read_exec_error(channel[0], &exec_error, &used);
    (void)close(channel[0]);

    /* Reap before examining any outcome, so that every later return path
       leaves no child behind. */
    int status;
    pid_t waited;
    do
    {
        waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited < 0)
    {
        /* Forbidden external reaping violates this narrow contract. */
        return errno;
    }
    if (read_error)
    {
        return read_error;
    }
    if (used != 0 && used != sizeof exec_error)
    {
        return EPROTO;
    }
    struct pl_launch result;
    if (used == sizeof exec_error && PL_MUTANT != 1)
    {
        result = (struct pl_launch){PL_EXEC_ERROR, exec_error};
    }
    else if (classify_status(status, &result) != 0)
    {
        return EPROTO;
    }
    if (PL_MUTANT == 2 && result.kind == PL_EXITED &&
        result.value == PL_CHILD_EXEC_FAILED)
    {
        result = (struct pl_launch){PL_EXEC_ERROR, ENOENT};
    }
    *out = result;
    return 0;
}

/* One worker's argument and result slot. The slot lives in the caller's
   frame, so a worker may only be abandoned after it has been joined. */
struct pool_work
{
    unsigned input;
    unsigned result;
};

static void *square(void *argument)
{
    struct pool_work *work = argument;
    work->result = work->input * work->input;
    return NULL;
}

/* Starts workers 1..workers. The 1-based worker number fail_ordinal (0 for
   none) is not started and reports EAGAIN, as pthread_create() would. Stops
   at the first failure. Returns how many threads were created, which is
   exactly the prefix that must be joined; *create_error is 0 or the error. */
static unsigned start_workers(unsigned workers, unsigned fail_ordinal,
                              pthread_t handles[PL_MAX_WORKERS],
                              struct pool_work work[PL_MAX_WORKERS],
                              int *create_error)
{
    unsigned created = 0;
    for (unsigned i = 0; i < workers; ++i)
    {
        work[i] = (struct pool_work){i + 1, 0};
        int error = fail_ordinal == i + 1 && PL_MUTANT != 3
                        ? EAGAIN
                        : pthread_create(&handles[i], NULL, square, &work[i]);
        if (error)
        {
            *create_error = error;
            break;
        }
        ++created;
    }
    return created;
}

/* Joins the created prefix and sums its results. */
static void join_workers(pthread_t handles[PL_MAX_WORKERS],
                         const struct pool_work work[PL_MAX_WORKERS],
                         struct pl_pool *result)
{
    for (unsigned i = 0; i < result->created; ++i)
    {
        int error = pthread_join(handles[i], NULL);
        /* With valid handles, no cancellation and exclusive join ownership, this
           join cannot fail under the declared API preconditions. Do not return
           stack-backed arguments while a borrower could remain live. */
        if (error)
        {
            _exit(PL_JOIN_FATAL_STATUS);
        }
        ++result->joined;
        ++result->completed;
        if (PL_MUTANT != 4 || i + 1 != result->created)
        {
            result->sum += work[i].result;
        }
    }
}

/* Contract: see CONTRACTS.md. Runs worker k (k = 1..workers) computing k*k
   on its own thread; at most PL_MAX_WORKERS. Every thread that was created
   is joined before return, so no thread outlives the stack storage it
   borrows. A creation failure is reported in out->create_error while the
   call itself returns 0 with the successful prefix accounted. On EINVAL *out
   is untouched. Blocks until the created threads finish. */
int pl_pool(unsigned workers, unsigned fail_ordinal, struct pl_pool *out)
{
    if (!out || workers > PL_MAX_WORKERS || fail_ordinal > workers)
    {
        return EINVAL;
    }
    pthread_t handles[PL_MAX_WORKERS];
    struct pool_work work[PL_MAX_WORKERS];
    struct pl_pool result = {0};
    result.created = start_workers(workers, fail_ordinal, handles, work,
                                   &result.create_error);
    join_workers(handles, work, &result);
    *out = result;
    return 0;
}
