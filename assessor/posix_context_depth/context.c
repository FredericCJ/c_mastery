#define _GNU_SOURCE
#include "context.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <unistd.h>

/* Reference implementation of the context.h contracts (see CONTRACTS.md).
 * PC_MUTANT selects one deliberate defect for the mutation campaign; 0 builds
 * the correct code. Each mutant is a single guarded change. */
#ifndef PC_MUTANT
#define PC_MUTANT 0
#endif

enum
{
    MUTANT_NONE = 0,
    MUTANT_LAUNCH_DROPS_ENV = 1,
    MUTANT_REPLACE_IGNORES_FSYNC_FAILURE = 2,
    MUTANT_REMAINING_UNDERFLOWS = 3,
    MUTANT_PARTIAL_REPORTED_SAFE = 4,
    MUTANT_REAP_RETIRES_ON_ANY_ERROR = 5
};

/* struct pc_launch.kind */
enum
{
    LAUNCH_EXITED = 1,
    LAUNCH_ERRNO = 2,
    LAUNCH_SIGNALED = 3
};

/* struct pc_ticket.phase */
enum
{
    TICKET_OWNED = 1,
    TICKET_COLLECTED = 2,
    TICKET_AUTHORITY_LOST = 3
};

/* struct pc_capability.backend */
enum
{
    BACKEND_UNPROBED = 0,
    BACKEND_EPOLL = 1,
    BACKEND_POLL = 2,
    BACKEND_UNSUPPORTED = 3
};

/* pc_operation_status() results */
enum
{
    OPERATION_COMPLETE = 0,
    OPERATION_SAFELY_UNSUPPORTED = 1,
    OPERATION_PARTIAL = 2
};

enum
{
    FIRST_NON_STDIO_FD = 3,
    CHILD_EXEC_FAILED_STATUS = 127,
    REPLACE_MAX_BYTES = 4096,
    REPLACE_LAST_FAIL_STAGE = 4
};

/* Stages of pc_replace(); a stage is reported only after it completed. */
enum
{
    STAGE_NONE = 0,
    STAGE_WRITTEN = 1,
    STAGE_SYNCED = 2,
    STAGE_PUBLISHED = 3,
    STAGE_DURABLE = 4
};

/* Fills out from a wait status. Returns 0, or -1 for a stop or continue
 * status that a blocking wait without WUNTRACED never reports. */
static int decode_status(int status, pid_t pid, struct pc_launch *out)
{
    out->pid = pid;
    if (WIFEXITED(status))
    {
        out->kind = LAUNCH_EXITED;
        out->value = WEXITSTATUS(status);
        return 0;
    }
    if (WIFSIGNALED(status))
    {
        out->kind = LAUNCH_SIGNALED;
        out->value = WTERMSIG(status);
        return 0;
    }
    return -1;
}

/* Reaps exactly pid, retrying only EINTR (no progress is lost by a retry).
 * Returns 0 and fills out, or a positive errno value. */
static int collect(pid_t pid, struct pc_launch *out)
{
    int status;
    pid_t waited;
    do
    {
        waited = waitpid(pid, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited < 0)
    {
        return errno;
    }
    if (decode_status(status, pid, out) != 0)
    {
        return EIO;
    }
    return 0;
}

int pc_reap(struct pc_ticket *ticket, pc_wait_fn wait_fn, void *context,
            struct pc_launch *result)
{
    if (!ticket || !result)
    {
        return EINVAL;
    }
    if (ticket->phase != TICKET_OWNED)
    {
        return EALREADY;
    }
    if (ticket->pid <= 0)
    {
        return EINVAL;
    }
    int status;
    pid_t waited;
    do
    {
        waited = wait_fn ? wait_fn(ticket->pid, &status, context)
                         : waitpid(ticket->pid, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited < 0)
    {
        int error = errno;
        /* ECHILD means another reaper already took the child: this ticket can
         * never collect it, so retire the authority. Any other failure leaves
         * the ticket owned so that the caller may retry. */
        if (error == ECHILD)
        {
            ticket->phase = TICKET_AUTHORITY_LOST;
            ticket->pid = 0;
        }
        else if (PC_MUTANT == MUTANT_REAP_RETIRES_ON_ANY_ERROR)
        {
            ticket->phase = TICKET_COLLECTED;
            ticket->pid = 0;
        }
        return error;
    }
    if (waited != ticket->pid)
    {
        return EPROTO;
    }
    /* Publish only after the status decoded: failures leave *result alone. */
    struct pc_launch collected = {0, 0, waited};
    if (decode_status(status, waited, &collected) != 0)
    {
        return EPROTO;
    }
    ticket->phase = TICKET_COLLECTED;
    ticket->pid = 0;
    *result = collected;
    return 0;
}

/* Writes all length bytes or gives up silently on the first hard error. Only
 * write() is called, so the child may use it between fork and _exit. */
static void write_fully(int fd, const void *buffer, size_t length)
{
    const unsigned char *bytes = buffer;
    size_t used = 0;
    while (used < length)
    {
        ssize_t written = write(fd, bytes + used, length - used);
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
            break;
        }
    }
}

/* Reads up to length bytes, stopping at end of file; *used counts the bytes
 * obtained. Returns 0, or the errno of a hard read failure. */
static int read_until_eof(int fd, void *buffer, size_t length, size_t *used)
{
    unsigned char *bytes = buffer;
    *used = 0;
    while (*used < length)
    {
        ssize_t got = read(fd, bytes + *used, length - *used);
        if (got > 0)
        {
            *used += (size_t)got;
        }
        else if (got == 0)
        {
            break;
        }
        else if (errno != EINTR)
        {
            return errno;
        }
    }
    return 0;
}

static int launch_arguments_invalid(const char *path, char *const argv[],
                                    char *const env[], int cwd, int output,
                                    const struct pc_launch *result)
{
    return !path || !argv || !env || !result || cwd < FIRST_NON_STDIO_FD ||
           output < FIRST_NON_STDIO_FD || cwd == output;
}

/* Child side of pc_launch: only prepared, async-signal-safe operations.
 * Returns the errno of the step that failed; returns nothing on success
 * because execve replaces the image. */
static int child_exec(const char *path, char *const argv[], char *const env[],
                      int cwd, int output)
{
    if (fchdir(cwd) < 0 || dup2(output, STDOUT_FILENO) < 0)
    {
        return errno;
    }
    if (output != STDOUT_FILENO)
    {
        close(output);
    }
    if (PC_MUTANT == MUTANT_LAUNCH_DROPS_ENV)
    {
        char *empty[] = {NULL};
        execve(path, argv, empty);
    }
    else
    {
        execve(path, argv, env);
    }
    return errno;
}

/* Launch protocol: a close-on-exec pipe carries the child's errno if exec (or
 * the setup before it) fails. A successful exec closes the pipe with no
 * bytes, so EOF alone is not proof of success; the wait status is still
 * collected and reported either way. */
int pc_launch(const char *path, char *const argv[], char *const env[], int cwd,
              int output, struct pc_launch *result)
{
    if (launch_arguments_invalid(path, argv, env, cwd, output, result))
    {
        return EINVAL;
    }
    int channel[2];
    if (pipe2(channel, O_CLOEXEC))
    {
        return errno;
    }
    pid_t pid = fork();
    if (pid < 0)
    {
        int error = errno;
        close(channel[0]);
        close(channel[1]);
        return error;
    }
    if (pid == 0)
    {
        close(channel[0]);
        int error = child_exec(path, argv, env, cwd, output);
        write_fully(channel[1], &error, sizeof error);
        _exit(CHILD_EXEC_FAILED_STATUS);
    }
    close(channel[1]);
    int child_error = 0;
    size_t record_bytes = 0;
    int read_error = read_until_eof(channel[0], &child_error,
                                    sizeof child_error, &record_bytes);
    close(channel[0]);
    struct pc_launch collected = {0, 0, 0};
    if (read_error)
    {
        (void)collect(pid, &collected);
        return read_error;
    }
    int collect_error = collect(pid, &collected);
    if (collect_error)
    {
        return collect_error;
    }
    if (record_bytes != 0 && record_bytes != sizeof child_error)
    {
        return EIO;
    }
    if (record_bytes != 0)
    {
        collected.kind = LAUNCH_ERRNO;
        collected.value = child_error;
    }
    *result = collected;
    return 0;
}

/* Builds the file actions that give the child its directory and stdout. */
static int add_launch_actions(posix_spawn_file_actions_t *actions, int cwd,
                              int output)
{
    int error = posix_spawn_file_actions_addfchdir_np(actions, cwd);
    if (!error)
    {
        error =
            posix_spawn_file_actions_adddup2(actions, output, STDOUT_FILENO);
    }
    if (!error && output != STDOUT_FILENO)
    {
        error = posix_spawn_file_actions_addclose(actions, output);
    }
    return error;
}

/* pc_launch also returns 0 with kind 2 (a child-reported exec failure). Here
 * kind 2 with return 0 covers a failed file-action setup and an immediate
 * posix_spawn failure; no child exists to collect in either case. */
int pc_spawn(const char *path, char *const argv[], char *const env[], int cwd,
             int output, struct pc_launch *result)
{
    if (launch_arguments_invalid(path, argv, env, cwd, output, result))
    {
        return EINVAL;
    }
    posix_spawn_file_actions_t actions;
    int error = posix_spawn_file_actions_init(&actions);
    if (error)
    {
        return error;
    }
    error = add_launch_actions(&actions, cwd, output);
    pid_t pid = 0;
    if (!error)
    {
        error = posix_spawn(&pid, path, &actions, NULL, argv, env);
    }
    (void)posix_spawn_file_actions_destroy(&actions);
    if (error)
    {
        *result = (struct pc_launch){LAUNCH_ERRNO, error, 0};
        return 0;
    }
    struct pc_launch collected = {0, 0, 0};
    error = collect(pid, &collected);
    if (!error)
    {
        *result = collected;
    }
    return error;
}

/* Progress of one pc_replace() call that cleanup must be able to undo. */
struct replace_run
{
    int temp_fd; /* -1 once closed */
    int temp_created; /* 1 from creation on; unlink only if unpublished */
    struct pc_replace result;
};

static void notify_stage(pc_step_fn step, void *context, unsigned stage)
{
    if (step)
    {
        step(context, stage);
    }
}

/* Writes the whole payload to the temporary file. Injected failure 1 models a
 * short write: one byte reaches the file, then EIO. Returns 0 or an errno. */
static int write_temporary(int fd, const unsigned char *data, size_t length,
                           unsigned fail_stage)
{
    size_t done = 0;
    while (done < length)
    {
        size_t want = length - done;
        if (fail_stage == 1 && want > 1)
        {
            want = 1;
        }
        ssize_t wrote = write(fd, data + done, want);
        if (wrote > 0)
        {
            done += (size_t)wrote;
        }
        else if (wrote < 0 && errno == EINTR)
        {
            continue;
        }
        else
        {
            return wrote == 0 ? EIO : errno;
        }
        if (fail_stage == 1)
        {
            return EIO;
        }
    }
    return fail_stage == 1 ? EIO : 0;
}

/* The temporary file must be on stable storage before it is renamed. */
static int sync_temporary(int fd, unsigned fail_stage)
{
    if (fail_stage == 2)
    {
        return PC_MUTANT == MUTANT_REPLACE_IGNORES_FSYNC_FAILURE ? 0 : EIO;
    }
    return fsync(fd) ? errno : 0;
}

/* Stages run in this order: write, fsync file, close, rename over "data"
 * (the point of visibility), fsync directory (the point of durability). The
 * lock is held by the caller. Returns the first error; run->result.stage is
 * the last completed stage. */
static int replace_locked(struct replace_run *run, int directory,
                          const unsigned char *data, size_t length,
                          unsigned fail_stage, pc_step_fn step, void *context)
{
    struct pc_replace *progress = &run->result;
    run->temp_fd = openat(directory, "temporary",
                          O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (run->temp_fd < 0)
    {
        return errno;
    }
    run->temp_created = 1;
    int error = write_temporary(run->temp_fd, data, length, fail_stage);
    if (error)
    {
        return error;
    }
    progress->stage = STAGE_WRITTEN;
    notify_stage(step, context, STAGE_WRITTEN);
    error = sync_temporary(run->temp_fd, fail_stage);
    if (error)
    {
        return error;
    }
    progress->stage = STAGE_SYNCED;
    notify_stage(step, context, STAGE_SYNCED);
    /* After close() the descriptor is gone even when close() reports an error;
     * forget it so cleanup never closes a reused number. */
    int fd = run->temp_fd;
    run->temp_fd = -1;
    if (close(fd))
    {
        return errno;
    }
    if (fail_stage == 3)
    {
        return EIO;
    }
    if (renameat(directory, "temporary", directory, "data"))
    {
        return errno;
    }
    progress->stage = STAGE_PUBLISHED;
    progress->visible = 1;
    notify_stage(step, context, STAGE_PUBLISHED);
    if (fail_stage == 4)
    {
        return EIO;
    }
    if (fsync(directory))
    {
        return errno;
    }
    progress->stage = STAGE_DURABLE;
    notify_stage(step, context, STAGE_DURABLE);
    return 0;
}

int pc_replace(int directory, int lock, const unsigned char *data,
               size_t length, unsigned fail_stage, pc_step_fn step,
               void *context, struct pc_replace *result)
{
    if (directory < 0 || lock < 0 || (!data && length) ||
        length > REPLACE_MAX_BYTES || fail_stage > REPLACE_LAST_FAIL_STAGE ||
        !result)
    {
        return EINVAL;
    }
    if (flock(lock, LOCK_EX))
    {
        return errno;
    }
    struct replace_run run = {-1, 0, {STAGE_NONE, 0, 0}};
    run.result.error = replace_locked(&run, directory, data, length, fail_stage,
                                      step, context);
    /* Cleanup keeps the first error. An unpublished temporary name is removed
     * so that a failed replacement leaves no stray file. */
    if (run.temp_fd >= 0 && close(run.temp_fd) && !run.result.error)
    {
        run.result.error = errno;
    }
    if (run.temp_created && run.result.stage < STAGE_PUBLISHED &&
        unlinkat(directory, "temporary", 0) && errno != ENOENT &&
        !run.result.error)
    {
        run.result.error = errno;
    }
    if (flock(lock, LOCK_UN) && !run.result.error)
    {
        run.result.error = errno;
    }
    *result = run.result;
    return run.result.error;
}

int pc_deadline(uint64_t now, uint64_t budget, uint64_t *deadline)
{
    if (!deadline)
    {
        return EINVAL;
    }
    if (budget > UINT64_MAX - now)
    {
        return ERANGE;
    }
    *deadline = now + budget;
    return 0;
}

uint64_t pc_remaining(uint64_t deadline, uint64_t now)
{
    if (PC_MUTANT == MUTANT_REMAINING_UNDERFLOWS)
    {
        return deadline - now;
    }
    return now >= deadline ? 0 : deadline - now;
}

int pc_choose(struct pc_capability *capability, unsigned epoch, int probe_error,
              int equivalent_poll, int operation_in_progress)
{
    if (!capability || !epoch || probe_error < 0)
    {
        return EINVAL;
    }
    /* Within one epoch a decision, success or failure, is final. */
    if (capability->backend != BACKEND_UNPROBED && capability->epoch == epoch)
    {
        return capability->error;
    }
    if (operation_in_progress)
    {
        return EBUSY;
    }
    capability->epoch = epoch;
    capability->error = 0;
    if (probe_error == 0)
    {
        capability->backend = BACKEND_EPOLL;
    }
    else if ((probe_error == ENOSYS || probe_error == EOPNOTSUPP) &&
             equivalent_poll)
    {
        capability->backend = BACKEND_POLL;
    }
    else
    {
        /* A real I/O error, or a missing facility with no equivalent
         * fallback, is preserved rather than hidden behind poll. */
        capability->backend = BACKEND_UNSUPPORTED;
        capability->error = probe_error;
    }
    return capability->error;
}

int pc_operation_status(int error, size_t completed)
{
    if (!error)
    {
        return OPERATION_COMPLETE;
    }
    if (completed)
    {
        return PC_MUTANT == MUTANT_PARTIAL_REPORTED_SAFE
                   ? OPERATION_SAFELY_UNSUPPORTED
                   : OPERATION_PARTIAL;
    }
    return OPERATION_SAFELY_UNSUPPORTED;
}

int pc_request(unsigned available, unsigned requested, int injected_error,
               unsigned *accepted)
{
    if (!accepted || !requested || injected_error < 0)
    {
        return EINVAL;
    }
    if (injected_error)
    {
        return injected_error;
    }
    if ((requested & available) != requested)
    {
        return EINVAL;
    }
    *accepted = requested;
    return 0;
}

/* Level-triggered, zero-timeout readiness through poll(). */
static int ready_by_poll(int fd)
{
    struct pollfd entry = {fd, POLLIN, 0};
    if (poll(&entry, 1, 0) < 0)
    {
        return -1;
    }
    if (entry.revents & POLLNVAL)
    {
        errno = EBADF;
        return -1;
    }
    return (entry.revents & (POLLIN | POLLHUP)) != 0;
}

/* The same question through a throwaway epoll instance, so no state is kept
 * between calls and a drained descriptor is reported as not ready. */
static int ready_by_epoll(int fd)
{
    int instance = epoll_create1(EPOLL_CLOEXEC);
    if (instance < 0)
    {
        return -1;
    }
    struct epoll_event event = {0};
    event.events = EPOLLIN;
    event.data.fd = fd;
    int count = -1;
    if (epoll_ctl(instance, EPOLL_CTL_ADD, fd, &event) == 0)
    {
        count = epoll_wait(instance, &event, 1, 0);
    }
    int saved_errno = errno;
    close(instance);
    errno = saved_errno;
    if (count < 0)
    {
        return -1;
    }
    return count > 0 && (event.events & (EPOLLIN | EPOLLHUP)) != 0;
}

int pc_ready(int backend, int fd)
{
    if (backend == BACKEND_POLL)
    {
        return ready_by_poll(fd);
    }
    if (backend == BACKEND_EPOLL)
    {
        return ready_by_epoll(fd);
    }
    errno = ENOTSUP;
    return -1;
}
