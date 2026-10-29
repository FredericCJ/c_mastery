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
#ifndef PC_MUTANT
#define PC_MUTANT 0
#endif
static int collect(pid_t pid, struct pc_launch *o)
{
    int status;
    pid_t r;
    do
    {
        r = waitpid(pid, &status, 0);
    } while (r < 0 && errno == EINTR);
    if (r < 0)
    {
        return errno;
    }
    o->pid = pid;
    if (WIFEXITED(status))
    {
        o->kind = 1;
        o->value = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        o->kind = 3;
        o->value = WTERMSIG(status);
    }
    else
    {
        return EIO;
    }
    return 0;
}

int pc_reap(struct pc_ticket *ticket, pc_wait_fn callback, void *context,
            struct pc_launch *out)
{
    int status;
    pid_t r;
    if (!ticket || !out)
    {
        return EINVAL;
    }
    if (ticket->phase != 1)
    {
        return EALREADY;
    }
    if (ticket->pid <= 0)
    {
        return EINVAL;
    }
    do
    {
        r = callback ? callback(ticket->pid, &status, context)
                     : waitpid(ticket->pid, &status, 0);
    } while (r < 0 && errno == EINTR);
    if (r < 0)
    {
        int e = errno;
        if (e == ECHILD)
        {
            ticket->phase = 3;
            ticket->pid = 0;
        }
        else if (PC_MUTANT == 5)
        {
            ticket->phase = 2;
            ticket->pid = 0;
        }
        return e;
    }
    if (r != ticket->pid)
    {
        return EPROTO;
    }
    struct pc_launch result = {0, 0, r};
    if (WIFEXITED(status))
    {
        result.kind = 1;
        result.value = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        result.kind = 3;
        result.value = WTERMSIG(status);
    }
    else
    {
        return EPROTO;
    }
    ticket->phase = 2;
    ticket->pid = 0;
    *out = result;
    return 0;
}

/* The child path calls only prepared, async-signal-safe operations. */
int pc_launch(const char *path, char *const argv[], char *const env[], int cwd,
              int output, struct pc_launch *out)
{
    int p[2], error = 0;
    size_t used = 0;
    pid_t pid;
    struct pc_launch value;
    if (!path || !argv || !env || !out || cwd < 3 || output < 3 ||
        cwd == output)
    {
        return EINVAL;
    }
    if (pipe2(p, O_CLOEXEC))
    {
        return errno;
    }
    pid = fork();
    if (pid < 0)
    {
        error = errno;
        close(p[0]);
        close(p[1]);
        return error;
    }
    if (pid == 0)
    {
        close(p[0]);
        if (fchdir(cwd) < 0 || dup2(output, STDOUT_FILENO) < 0)
        {
            error = errno;
        }
        else
        {
            if (output != STDOUT_FILENO)
            {
                close(output);
            }
            if (PC_MUTANT == 1)
            {
                char *empty[] = {NULL};
                execve(path, argv, empty);
            }
            else
            {
                execve(path, argv, env);
            }
            error = errno;
        }
        while (used < sizeof error)
        {
            ssize_t n = write(p[1], (const unsigned char *)&error + used,
                              sizeof error - used);
            if (n > 0)
            {
                used += (size_t)n;
            }
            else if (n < 0 && errno == EINTR)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        _exit(127);
    }
    close(p[1]);
    while (used < sizeof error)
    {
        ssize_t n =
            read(p[0], (unsigned char *)&error + used, sizeof error - used);
        if (n > 0)
        {
            used += (size_t)n;
        }
        else if (n == 0)
        {
            break;
        }
        else if (errno != EINTR)
        {
            int saved = errno;
            close(p[0]);
            (void)collect(pid, &value);
            return saved;
        }
    }
    close(p[0]);
    {
        int r = collect(pid, &value);
        if (r)
        {
            return r;
        }
    }
    if (used && used != sizeof error)
    {
        return EIO;
    }
    if (used)
    {
        value.kind = 2;
        value.value = error;
    }
    *out = value;
    return 0;
}

int pc_spawn(const char *path, char *const argv[], char *const env[], int cwd,
             int output, struct pc_launch *out)
{
    posix_spawn_file_actions_t a;
    pid_t pid;
    struct pc_launch value;
    int r;
    if (!path || !argv || !env || !out || cwd < 3 || output < 3 ||
        cwd == output)
    {
        return EINVAL;
    }
    r = posix_spawn_file_actions_init(&a);
    if (r)
    {
        return r;
    }
    r = posix_spawn_file_actions_addfchdir_np(&a, cwd);
    if (!r)
    {
        r = posix_spawn_file_actions_adddup2(&a, output, STDOUT_FILENO);
    }
    if (!r && output != STDOUT_FILENO)
    {
        r = posix_spawn_file_actions_addclose(&a, output);
    }
    if (!r)
    {
        r = posix_spawn(&pid, path, &a, NULL, argv, env);
    }
    (void)posix_spawn_file_actions_destroy(&a);
    if (r)
    {
        *out = (struct pc_launch){2, r, 0};
        return 0;
    }
    r = collect(pid, &value);
    if (!r)
    {
        *out = value;
    }
    return r;
}

int pc_replace(int dir, int lock, const unsigned char *data, size_t n,
               unsigned failure, pc_step_fn step, void *ctx,
               struct pc_replace *out)
{
    struct pc_replace r = {0, 0, 0};
    int fd = -1, created = 0;
    size_t done = 0;
    if (dir < 0 || lock < 0 || (!data && n) || n > 4096 || failure > 4 || !out)
    {
        return EINVAL;
    }
    if (flock(lock, LOCK_EX))
    {
        return errno;
    }
    fd =
        openat(dir, "temporary", O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0)
    {
        r.error = errno;
        goto finish;
    }
    created = 1;
    while (done < n)
    {
        size_t want = n - done;
        if (failure == 1 && want > 1)
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
            r.error = wrote == 0 ? EIO : errno;
            goto finish;
        }
        if (failure == 1)
        {
            r.error = EIO;
            goto finish;
        }
    }
    if (failure == 1)
    {
        r.error = EIO;
        goto finish;
    }
    r.stage = 1;
    if (step)
    {
        step(ctx, 1);
    }
    if (failure == 2)
    {
        if (PC_MUTANT != 2)
        {
            r.error = EIO;
            goto finish;
        }
    }
    else if (fsync(fd))
    {
        r.error = errno;
        goto finish;
    }
    r.stage = 2;
    if (step)
    {
        step(ctx, 2);
    }
    if (close(fd))
    {
        fd = -1;
        r.error = errno;
        goto finish;
    }
    fd = -1;
    if (failure == 3)
    {
        r.error = EIO;
        goto finish;
    }
    if (renameat(dir, "temporary", dir, "data"))
    {
        r.error = errno;
        goto finish;
    }
    r.stage = 3;
    r.visible = 1;
    if (step)
    {
        step(ctx, 3);
    }
    if (failure == 4)
    {
        r.error = EIO;
        goto finish;
    }
    if (fsync(dir))
    {
        r.error = errno;
        goto finish;
    }
    r.stage = 4;
    if (step)
    {
        step(ctx, 4);
    }
finish:
    if (fd >= 0 && close(fd) && !r.error)
    {
        r.error = errno;
    }
    if (created && r.stage < 3 && unlinkat(dir, "temporary", 0) &&
        errno != ENOENT && !r.error)
    {
        r.error = errno;
    }
    if (flock(lock, LOCK_UN) && !r.error)
    {
        r.error = errno;
    }
    *out = r;
    return r.error;
}

int pc_deadline(uint64_t now, uint64_t budget, uint64_t *out)
{
    if (!out)
    {
        return EINVAL;
    }
    if (budget > UINT64_MAX - now)
    {
        return ERANGE;
    }
    *out = now + budget;
    return 0;
}

uint64_t pc_remaining(uint64_t d, uint64_t n)
{
    if (PC_MUTANT == 3)
    {
        return d - n;
    }
    return n >= d ? 0 : d - n;
}

int pc_choose(struct pc_capability *c, unsigned epoch, int probe,
              int equivalent, int in_progress)
{
    if (!c || !epoch || probe < 0)
    {
        return EINVAL;
    }
    if (c->backend && c->epoch == epoch)
    {
        return c->error;
    }
    if (in_progress)
    {
        return EBUSY;
    }
    c->epoch = epoch;
    c->error = 0;
    if (probe == 0)
    {
        c->backend = 1;
    }
    else if ((probe == ENOSYS || probe == EOPNOTSUPP) && equivalent)
    {
        c->backend = 2;
    }
    else
    {
        c->backend = 3;
        c->error = probe;
    }
    return c->error;
}

int pc_operation_status(int error, size_t completed)
{
    if (!error)
    {
        return 0;
    }
    if (completed)
    {
        return PC_MUTANT == 4 ? 1 : 2;
    }
    return 1;
}

int pc_request(unsigned available, unsigned requested, int error,
               unsigned *accepted)
{
    if (!accepted || !requested || error < 0)
    {
        return EINVAL;
    }
    if (error)
    {
        return error;
    }
    if ((requested & available) != requested)
    {
        return EINVAL;
    }
    *accepted = requested;
    return 0;
}

int pc_ready(int backend, int fd)
{
    if (backend == 2)
    {
        struct pollfd p = {fd, POLLIN, 0};
        int r = poll(&p, 1, 0);
        if (r < 0)
        {
            return -1;
        }
        if (p.revents & POLLNVAL)
        {
            errno = EBADF;
            return -1;
        }
        return (p.revents & (POLLIN | POLLHUP)) != 0;
    }
    if (backend == 1)
    {
        struct epoll_event event = {0};
        int ep = epoll_create1(EPOLL_CLOEXEC);
        if (ep < 0)
        {
            return -1;
        }
        event.events = EPOLLIN;
        event.data.fd = fd;
        if (epoll_ctl(ep, EPOLL_CTL_ADD, fd, &event))
        {
            int e = errno;
            close(ep);
            errno = e;
            return -1;
        }
        int r = epoll_wait(ep, &event, 1, 0);
        int e = errno;
        close(ep);
        errno = e;
        return r < 0 ? -1
                     : (r > 0 && (event.events & (EPOLLIN | EPOLLHUP)) != 0);
    }
    errno = ENOTSUP;
    return -1;
}
