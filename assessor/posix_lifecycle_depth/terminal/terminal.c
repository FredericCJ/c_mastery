#define _GNU_SOURCE
#include "terminal.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#ifndef PT_MUTANT
#define PT_MUTANT 0
#endif
void pt_guard_init(struct pt_guard *g)
{
    if (g)
    {
        *g = (struct pt_guard){.fd = -1};
    }
}

bool pt_attributes_equal(const struct termios *a, const struct termios *b)
{
    if (a->c_iflag != b->c_iflag || a->c_oflag != b->c_oflag ||
        a->c_cflag != b->c_cflag || a->c_lflag != b->c_lflag ||
        cfgetispeed(a) != cfgetispeed(b) || cfgetospeed(a) != cfgetospeed(b))
    {
        return false;
    }
    for (size_t i = 0; i < NCCS; ++i)
    {
        if (a->c_cc[i] != b->c_cc[i])
        {
            return false;
        }
    }
    return true;
}

int pt_enter(struct pt_guard *g, int fd, enum pt_mode mode, unsigned minimum,
             unsigned deciseconds)
{
    struct termios wanted, actual;
    if (!g || g->restore_pending || (mode != PT_RAW && mode != PT_CANONICAL) ||
        minimum > UCHAR_MAX || deciseconds > UCHAR_MAX)
    {
        return EINVAL;
    }
    if (tcgetattr(fd, &wanted) < 0)
    {
        return errno;
    }
    g->saved = wanted;
    g->fd = fd;
    g->saved_valid = true;
    wanted.c_iflag &=
        ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    wanted.c_oflag &= ~OPOST;
    wanted.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    wanted.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    wanted.c_cflag |= CS8 | CREAD | CLOCAL;
    if (mode == PT_CANONICAL)
    {
        wanted.c_lflag |= ICANON;
    }
    wanted.c_cc[VMIN] = (cc_t)minimum;
    wanted.c_cc[VTIME] = (cc_t)deciseconds;
    if (cfsetispeed(&wanted, B9600) < 0 || cfsetospeed(&wanted, B9600) < 0)
    {
        return errno;
    }
    g->restore_pending = true;
    if (tcsetattr(fd, TCSANOW, &wanted) < 0)
    {
        return errno;
    }
    if (tcgetattr(fd, &actual) < 0)
    {
        return errno;
    }
#if PT_MUTANT != 1
    if (!pt_attributes_equal(&wanted, &actual))
    {
        return EIO;
    }
#endif
    return 0;
}

int pt_restore(struct pt_guard *g)
{
    if (!g)
    {
        return EINVAL;
    }
    if (!g->restore_pending)
    {
        return 0;
    }
    if (!g->saved_valid)
    {
        return EINVAL;
    }
#if PT_MUTANT == 2
    g->restore_pending = false;
    return 0;
#else
    struct termios actual;
    if (tcsetattr(g->fd, TCSANOW, &g->saved) < 0)
    {
        return errno;
    }
    if (tcgetattr(g->fd, &actual) < 0)
    {
        return errno;
    }
    if (!pt_attributes_equal(&g->saved, &actual))
    {
        return EIO;
    }
    g->restore_pending = false;
    return 0;
#endif
}

static long long now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) < 0)
    {
        return -1;
    }
    return (long long)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

static int send_all(int fd, const unsigned char *b, size_t n)
{
    size_t done = 0;
    while (done < n)
    {
        ssize_t r = write(fd, b + done, n - done);
        if (r < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return errno;
        }
        if (!r)
        {
            return EIO;
        }
        done += (size_t)r;
    }
    return 0;
}

static void close_if(int *fd)
{
    if (*fd >= 0)
    {
        close(*fd);
        *fd = -1;
    }
}

static void reap_all(struct pt_pipeline *r)
{
    for (unsigned i = 0; i < 3; ++i)
    {
        if (r->children[i] > 0 && !(r->reaped_mask & (1u << i)))
        {
            pid_t p;
            do
            {
                p = waitpid(r->children[i], &r->status[i], 0);
            } while (p < 0 && errno == EINTR);
            if (p == r->children[i])
            {
                r->reaped_mask |= 1u << i;
            }
        }
    }
}

int pt_pipeline(size_t bytes, bool early, unsigned timeout,
                struct pt_pipeline *out)
{
    int pipes[3][2] = {{-1, -1}, {-1, -1}, {-1, -1}}, error = 0;
    struct pt_pipeline r = {0};
    long long start;
    if (!out || bytes > 1024 * 1024 || timeout == 0 || timeout > 10000)
    {
        return EINVAL;
    }
    start = now();
    if (start < 0)
    {
        return errno;
    }
    for (unsigned i = 0; i < 3; ++i)
    {
        if (pipe2(pipes[i], O_CLOEXEC) < 0)
        {
            error = errno;
            goto cleanup;
        }
    }
    for (unsigned stage = 0; stage < 3; ++stage)
    {
        pid_t child = fork();
        if (child < 0)
        {
            error = errno;
            goto cleanup;
        }
        if (child == 0)
        {
            struct sigaction action = {0};
            action.sa_handler = SIG_IGN;
            sigemptyset(&action.sa_mask);
            if (sigaction(SIGPIPE, &action, NULL) < 0)
            {
                _exit(90);
            }
            int input = stage ? pipes[stage - 1][0] : -1,
                output = stage < 2 ? pipes[stage][1] : pipes[2][1];
            for (unsigned i = 0; i < 3; ++i)
            {
                for (unsigned j = 0; j < 2; ++j)
                {
                    if (pipes[i][j] != input && pipes[i][j] != output)
                    {
                        close(pipes[i][j]);
                    }
                }
            }
            unsigned char b[512];
            size_t total = 0;
            uint32_t sum = 0;
            if (stage == 0)
            {
                while (total < bytes)
                {
                    size_t n = bytes - total;
                    if (n > sizeof b)
                    {
                        n = sizeof b;
                    }
                    for (size_t i = 0; i < n; ++i)
                    {
                        b[i] = (unsigned char)((total + i) % 251);
                    }
                    int e = send_all(output, b, n);
                    if (e)
                    {
                        _exit(e == EPIPE ? 20 : 91);
                    }
                    total += n;
                }
                _exit(0);
            }
            for (;;)
            {
                ssize_t n = read(input, b, sizeof b);
                if (n < 0)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    _exit(92);
                }
                if (!n)
                {
                    break;
                }
                if (stage == 1)
                {
                    int e = send_all(output, b, (size_t)n);
                    if (e)
                    {
                        _exit(e == EPIPE ? 20 : 93);
                    }
                }
                else
                {
                    for (ssize_t i = 0; i < n; ++i)
                    {
                        sum += b[i];
                    }
                    total += (size_t)n;
                    if (early)
                    {
                        break;
                    }
                }
            }
            if (stage == 2)
            {
                struct
                {
                    size_t bytes;
                    uint32_t sum;
                } value = {total, sum};

                if (send_all(output, (const unsigned char *)&value,
                             sizeof value))
                {
                    _exit(94);
                }
            }
            _exit(0);
        }
        r.children[stage] = child;
    }
    for (unsigned i = 0; i < 3; ++i)
    {
        for (unsigned j = 0; j < 2; ++j)
        {
            if (!(i == 2 && j == 0))
            {
#if PT_MUTANT == 3
                if (i == 1 && j == 1)
                {
                    continue;
                }
#endif
                close_if(&pipes[i][j]);
            }
        }
    }
    while (r.reaped_mask != 7)
    {
        for (unsigned i = 0; i < 3; ++i)
        {
            if (!(r.reaped_mask & (1u << i)))
            {
#if PT_MUTANT == 4
                siginfo_t info = {0};
                int result = waitid(P_PID, (id_t)r.children[i], &info,
                                    WEXITED | WNOHANG | WNOWAIT);
                if (result == 0 && info.si_pid == r.children[i])
                {
                    r.status[i] = info.si_code == CLD_EXITED
                                      ? info.si_status << 8
                                      : info.si_status;
                    r.reaped_mask |= 1u << i;
                }
                else if (result < 0 && errno != EINTR)
                {
                    error = errno;
                    goto cleanup;
                }
#else
                pid_t p = waitpid(r.children[i], &r.status[i], WNOHANG);
                if (p == r.children[i])
                {
                    r.reaped_mask |= 1u << i;
                }
                else if (p < 0 && errno != EINTR)
                {
                    error = errno;
                    goto cleanup;
                }
#endif
            }
        }
        if (r.reaped_mask == 7)
        {
            break;
        }
        long long current = now();
        if (current < 0)
        {
            error = errno;
            goto cleanup;
        }
        if (current - start >= (long long)timeout)
        {
            error = ETIMEDOUT;
            goto cleanup;
        }
        (void)poll(NULL, 0, 1);
    }
    {
        struct
        {
            size_t bytes;
            uint32_t sum;
        } value = {0};

        unsigned char *b = (unsigned char *)&value;
        size_t got = 0;
        while (got < sizeof value)
        {
            ssize_t n = read(pipes[2][0], b + got, sizeof value - got);
            if (n < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                error = errno;
                break;
            }
            if (!n)
            {
                error = EPROTO;
                break;
            }
            got += (size_t)n;
        }
        if (!error)
        {
            r.bytes = value.bytes;
            r.sum = value.sum;
        }
    }
cleanup:
    for (unsigned i = 0; i < 3; ++i)
    {
        for (unsigned j = 0; j < 2; ++j)
        {
            close_if(&pipes[i][j]);
        }
    }
    if (error)
    {
        for (unsigned i = 0; i < 3; ++i)
        {
            if (r.children[i] > 0 && !(r.reaped_mask & (1u << i)))
            {
                kill(r.children[i], SIGKILL);
            }
        }
    }
    reap_all(&r);
    *out = r;
    return error;
}
