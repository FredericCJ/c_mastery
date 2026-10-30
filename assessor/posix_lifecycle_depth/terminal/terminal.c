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

/* The pipeline is producer (stage 0) -> copier (stage 1) -> sink (stage 2),
   joined by one pipe each: pipes[s] carries stage s's output. The sink's
   pipe, pipes[PT_SINK_STAGE], carries the final result back to the parent. */
enum
{
    PT_STAGES = 3,
    PT_PRODUCER_STAGE = 0,
    PT_COPIER_STAGE = 1,
    PT_SINK_STAGE = 2,
    PT_ALL_REAPED_MASK = (1 << PT_STAGES) - 1
};

enum
{
    PT_MAX_PIPELINE_BYTES = 1024 * 1024,
    PT_MAX_DEADLINE_MS = 10000,
    PT_CHUNK_BYTES = 512, /* size of every read/write in a stage */
    PT_PATTERN_PERIOD = 251 /* does not divide the chunk size, so successive
                               chunks start at different pattern phases; the
                               pattern realigns with a chunk boundary only
                               every lcm(251, 512) = 128512 bytes */
};

/* Wait-loop poll interval. The loop must poll: SIGCHLD is not blocked or
   handled here, because signal policy changes happen only in children. */
enum
{
    PT_POLL_INTERVAL_MS = 1,
    PT_MS_PER_SECOND = 1000,
    PT_NS_PER_MS = 1000000
};

/* Child exit statuses. Anything other than 0 or PT_EXIT_PEER_CLOSED is a
   failure of the stage itself. */
enum
{
    PT_EXIT_PEER_CLOSED = 20, /* EPIPE: the reader went away; expected when
                                 the sink stops early */
    PT_EXIT_SIGPIPE_SETUP = 90,
    PT_EXIT_PRODUCER_WRITE = 91,
    PT_EXIT_READ = 92,
    PT_EXIT_COPIER_WRITE = 93,
    PT_EXIT_SINK_RESULT = 94
};

/* What the sink sends to the parent: native-endian, same binary on both
   sides, so the struct is sent as raw bytes. (Its tail padding is sent too
   and is never read.) */
struct pipeline_result
{
    size_t bytes;
    uint32_t sum;
};

void pt_guard_init(struct pt_guard *g)
{
    if (g)
    {
        *g = (struct pt_guard){.fd = -1};
    }
}

/* Compares c_iflag, c_oflag, c_cflag, c_lflag, the input and output speeds
   as reported by cfgetispeed()/cfgetospeed(), and every c_cc entry. c_line
   is not compared. On Linux the speed is encoded in c_cflag, so the speed
   calls are an additional, not independent, check. */
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

/* Turns `attributes` into 8N1 raw mode (the cfmakeraw set), then re-enables
   canonical input for PT_CANONICAL. VMIN and VTIME are in characters and
   deciseconds. Returns 0 or the errno of the baud-rate calls. */
static int make_attributes(struct termios *attributes, enum pt_mode mode,
                           unsigned minimum_chars, unsigned deciseconds)
{
    attributes->c_iflag &=
        ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    attributes->c_oflag &= ~OPOST;
    attributes->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    attributes->c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    attributes->c_cflag |= CS8 | CREAD | CLOCAL;
    if (mode == PT_CANONICAL)
    {
        attributes->c_lflag |= ICANON;
    }
    attributes->c_cc[VMIN] = (cc_t)minimum_chars;
    attributes->c_cc[VTIME] = (cc_t)deciseconds;
    if (cfsetispeed(attributes, B9600) < 0 ||
        cfsetospeed(attributes, B9600) < 0)
    {
        return errno;
    }
    return 0;
}

/* Contract: see CONTRACTS.md. The guard borrows `fd`. The original
   attributes are saved before anything changes. restore_pending is set
   BEFORE tcsetattr(), because a failed or partial tcsetattr() may still
   have altered the terminal, and the caller must then call pt_restore().
   The new attributes are read back and compared: tcsetattr() reports
   success if any one requested change was made. */
int pt_enter(struct pt_guard *g, int fd, enum pt_mode mode, unsigned minimum,
             unsigned deciseconds)
{
    if (!g || g->restore_pending || (mode != PT_RAW && mode != PT_CANONICAL) ||
        minimum > UCHAR_MAX || deciseconds > UCHAR_MAX)
    {
        return EINVAL;
    }
    struct termios wanted;
    if (tcgetattr(fd, &wanted) < 0)
    {
        return errno;
    }
    g->saved = wanted;
    g->fd = fd;
    g->saved_valid = true;
    int error = make_attributes(&wanted, mode, minimum, deciseconds);
    if (error)
    {
        return error;
    }
    g->restore_pending = true;
    if (tcsetattr(fd, TCSANOW, &wanted) < 0)
    {
        return errno;
    }
    struct termios actual;
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

/* Puts the saved attributes back and verifies by read-back. Idempotent when
   nothing is pending. On any failure restore_pending stays true so that the
   caller can retry. */
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

/* Monotonic milliseconds, or -1 with errno set. */
static long long now_ms(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) < 0)
    {
        return -1;
    }
    return (long long)t.tv_sec * PT_MS_PER_SECOND + t.tv_nsec / PT_NS_PER_MS;
}

static int send_all(int fd, const unsigned char *bytes, size_t length)
{
    size_t done = 0;
    while (done < length)
    {
        ssize_t written = write(fd, bytes + done, length - done);
        if (written < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return errno;
        }
        if (written == 0)
        {
            return EIO;
        }
        done += (size_t)written;
    }
    return 0;
}

/* Closes and forgets a descriptor, so a second call is harmless. */
static void close_if(int *fd)
{
    if (*fd >= 0)
    {
        close(*fd);
        *fd = -1;
    }
}

static bool is_reaped(const struct pt_pipeline *r, unsigned stage)
{
    return (r->reaped_mask & (1u << stage)) != 0;
}

/* Blocks until every child that was created and not yet reaped is reaped.
   A child that cannot be reaped (waitpid failure other than EINTR) stays
   unreaped in reaped_mask, which tells the caller exactly what leaked. */
static void reap_all(struct pt_pipeline *r)
{
    for (unsigned stage = 0; stage < PT_STAGES; ++stage)
    {
        if (r->children[stage] <= 0 || is_reaped(r, stage))
        {
            continue;
        }
        pid_t reaped;
        do
        {
            reaped = waitpid(r->children[stage], &r->status[stage], 0);
        } while (reaped < 0 && errno == EINTR);
        if (reaped == r->children[stage])
        {
            r->reaped_mask |= 1u << stage;
        }
    }
}

/* Stage 0: writes `bytes` bytes of the repeating pattern i % 251. */
static _Noreturn void run_producer(int output, size_t bytes)
{
    unsigned char chunk[PT_CHUNK_BYTES];
    size_t total = 0;
    while (total < bytes)
    {
        size_t n = bytes - total;
        if (n > sizeof chunk)
        {
            n = sizeof chunk;
        }
        for (size_t i = 0; i < n; ++i)
        {
            chunk[i] = (unsigned char)((total + i) % PT_PATTERN_PERIOD);
        }
        int error = send_all(output, chunk, n);
        if (error)
        {
            _exit(error == EPIPE ? PT_EXIT_PEER_CLOSED
                                 : PT_EXIT_PRODUCER_WRITE);
        }
        total += n;
    }
    _exit(0);
}

/* Stage 1: forwards every byte until its input reaches EOF. */
static _Noreturn void run_copier(int input, int output)
{
    unsigned char chunk[PT_CHUNK_BYTES];
    for (;;)
    {
        ssize_t n = read(input, chunk, sizeof chunk);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            _exit(PT_EXIT_READ);
        }
        if (n == 0)
        {
            break;
        }
        int error = send_all(output, chunk, (size_t)n);
        if (error)
        {
            _exit(error == EPIPE ? PT_EXIT_PEER_CLOSED : PT_EXIT_COPIER_WRITE);
        }
    }
    _exit(0);
}

/* Stage 2: counts and sums its input, then reports to the parent. With
   `early` it stops after the first non-empty read and closes its input on
   exit, so the upstream stages see EPIPE (SIGPIPE is ignored in children). */
static _Noreturn void run_sink(int input, int output, bool early)
{
    unsigned char chunk[PT_CHUNK_BYTES];
    struct pipeline_result value = {0, 0};
    for (;;)
    {
        ssize_t n = read(input, chunk, sizeof chunk);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            _exit(PT_EXIT_READ);
        }
        if (n == 0)
        {
            break;
        }
        for (ssize_t i = 0; i < n; ++i)
        {
            value.sum += chunk[i];
        }
        value.bytes += (size_t)n;
        if (early)
        {
            break;
        }
    }
    if (send_all(output, (const unsigned char *)&value, sizeof value))
    {
        _exit(PT_EXIT_SINK_RESULT);
    }
    _exit(0);
}

/* Child after fork(): ignore SIGPIPE (a reader that closes early must
   surface as EPIPE, not kill the stage), close every pipe end this stage
   does not use, then run the stage. Closing the unused ends is what lets
   EOF and EPIPE propagate: a stray write end held here would keep the
   downstream reader waiting forever. Never returns. */
static _Noreturn void run_stage(unsigned stage, int pipes[PT_STAGES][2],
                                size_t bytes, bool early)
{
    struct sigaction action = {0};
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGPIPE, &action, NULL) < 0)
    {
        _exit(PT_EXIT_SIGPIPE_SETUP);
    }
    int input = stage > 0 ? pipes[stage - 1][0] : -1;
    int output = pipes[stage][1];
    for (unsigned i = 0; i < PT_STAGES; ++i)
    {
        for (unsigned end = 0; end < 2; ++end)
        {
            if (pipes[i][end] != input && pipes[i][end] != output)
            {
                close(pipes[i][end]);
            }
        }
    }
    if (stage == PT_PRODUCER_STAGE)
    {
        run_producer(output, bytes);
    }
    if (stage == PT_COPIER_STAGE)
    {
        run_copier(input, output);
    }
    run_sink(input, output, early);
}

/* Parent: close every end except the one it reads the result from, so that
   the children hold the only other references and EOF can happen. */
static void close_parent_ends(int pipes[PT_STAGES][2])
{
    for (unsigned i = 0; i < PT_STAGES; ++i)
    {
        for (unsigned end = 0; end < 2; ++end)
        {
            if (i == PT_SINK_STAGE && end == 0)
            {
                continue;
            }
#if PT_MUTANT == 3
            if (i == 1 && end == 1)
            {
                continue;
            }
#endif
            close_if(&pipes[i][end]);
        }
    }
}

/* Checks one child without blocking and records it in r. Returns 0 or the
   errno of a wait failure. */
static int poll_child(struct pt_pipeline *r, unsigned stage)
{
#if PT_MUTANT == 4
    /* Mutant: peek with WNOWAIT, so the child is never actually reaped. */
    siginfo_t info = {0};
    int result = waitid(P_PID, (id_t)r->children[stage], &info,
                        WEXITED | WNOHANG | WNOWAIT);
    if (result == 0 && info.si_pid == r->children[stage])
    {
        r->status[stage] =
            info.si_code == CLD_EXITED ? info.si_status << 8 : info.si_status;
        r->reaped_mask |= 1u << stage;
    }
    else if (result < 0 && errno != EINTR)
    {
        return errno;
    }
#else
    pid_t reaped = waitpid(r->children[stage], &r->status[stage], WNOHANG);
    if (reaped == r->children[stage])
    {
        r->reaped_mask |= 1u << stage;
    }
    else if (reaped < 0 && errno != EINTR)
    {
        return errno;
    }
#endif
    return 0;
}

/* Polls the children until all are reaped or `deadline_ms` has elapsed since
   `start_ms`. Returns 0, ETIMEDOUT or an errno. Children that are still
   running at the deadline are left for the caller to kill. */
static int wait_for_children(struct pt_pipeline *r, long long start_ms,
                             unsigned deadline_ms)
{
    while (r->reaped_mask != PT_ALL_REAPED_MASK)
    {
        for (unsigned stage = 0; stage < PT_STAGES; ++stage)
        {
            if (is_reaped(r, stage))
            {
                continue;
            }
            int error = poll_child(r, stage);
            if (error)
            {
                return error;
            }
        }
        if (r->reaped_mask == PT_ALL_REAPED_MASK)
        {
            break;
        }
        long long current_ms = now_ms();
        if (current_ms < 0)
        {
            return errno;
        }
        if (current_ms - start_ms >= (long long)deadline_ms)
        {
            return ETIMEDOUT;
        }
        (void)poll(NULL, 0, PT_POLL_INTERVAL_MS);
    }
    return 0;
}

/* Reads the sink's fixed-size result from the parent's end of the last pipe.
   EOF before a full record is EPROTO. Returns 0 or an errno. */
static int read_result(int fd, struct pipeline_result *value)
{
    unsigned char *raw = (unsigned char *)value;
    size_t got = 0;
    while (got < sizeof *value)
    {
        ssize_t n = read(fd, raw + got, sizeof *value - got);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return errno;
        }
        if (n == 0)
        {
            return EPROTO;
        }
        got += (size_t)n;
    }
    return 0;
}

/* The steps of pt_pipeline() that can fail, in order. Every resource it
   acquires is recorded in `pipes` and `r` before the next step can fail, so
   the caller's cleanup is the same on every path. */
static int run_pipeline(int pipes[PT_STAGES][2], struct pt_pipeline *r,
                        size_t bytes, bool early, unsigned deadline_ms,
                        long long start_ms)
{
    for (unsigned i = 0; i < PT_STAGES; ++i)
    {
        if (pipe2(pipes[i], O_CLOEXEC) < 0)
        {
            return errno;
        }
    }
    for (unsigned stage = 0; stage < PT_STAGES; ++stage)
    {
        pid_t child = fork();
        if (child < 0)
        {
            return errno;
        }
        if (child == 0)
        {
            run_stage(stage, pipes, bytes, early);
        }
        r->children[stage] = child;
    }
    close_parent_ends(pipes);
    int error = wait_for_children(r, start_ms, deadline_ms);
    if (error)
    {
        return error;
    }
    struct pipeline_result value = {0, 0};
    error = read_result(pipes[PT_SINK_STAGE][0], &value);
    if (!error)
    {
        r->bytes = value.bytes;
        r->sum = value.sum;
    }
    return error;
}

/* Contract: see terminal.h. On every return all pipe ends the function
   opened are closed. Every child it created is reaped (a still-running
   child is killed first), except one whose waitpid() fails with an error
   other than EINTR: that child stays unreaped and its bit is clear in
   reaped_mask. *out reports the children, wait statuses and result found so
   far. Blocks up to deadline_ms (milliseconds). */
int pt_pipeline(size_t bytes, bool early_sink, unsigned deadline_ms,
                struct pt_pipeline *out)
{
    if (!out || bytes > PT_MAX_PIPELINE_BYTES || deadline_ms == 0 ||
        deadline_ms > PT_MAX_DEADLINE_MS)
    {
        return EINVAL;
    }
    long long start_ms = now_ms();
    if (start_ms < 0)
    {
        return errno;
    }
    int pipes[PT_STAGES][2] = {{-1, -1}, {-1, -1}, {-1, -1}};
    struct pt_pipeline r = {0};
    int error =
        run_pipeline(pipes, &r, bytes, early_sink, deadline_ms, start_ms);

    /* Cleanup order matters: close our pipe ends first (so blocked children
       see EOF/EPIPE), kill only after a failure, then reap. */
    for (unsigned i = 0; i < PT_STAGES; ++i)
    {
        close_if(&pipes[i][0]);
        close_if(&pipes[i][1]);
    }
    if (error)
    {
        for (unsigned stage = 0; stage < PT_STAGES; ++stage)
        {
            if (r.children[stage] > 0 && !is_reaped(&r, stage))
            {
                kill(r.children[stage], SIGKILL);
            }
        }
    }
    reap_all(&r);
    *out = r;
    return error;
}
