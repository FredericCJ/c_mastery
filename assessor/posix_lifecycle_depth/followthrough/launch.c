#define _GNU_SOURCE
#include "launch.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#ifndef PLX_MUTANT
#define PLX_MUTANT 0
#endif

/* Largest transfer plx_read_all()/plx_write_all() accept: the protocol only
   ever moves a handful of bytes, so a bound keeps the loops easy to reason
   about. */
enum
{
    PLX_MAX_TRANSFER_BYTES = 8
};

/* Wire format of the child-to-parent record (see launch.h): two little-endian
   uint32 fields, stage then errno. */
enum
{
    PLX_FIELD_BYTES = 4,
    PLX_RECORD_BYTES = 2 * PLX_FIELD_BYTES
};

/* Stage numbers carried in the record. */
enum
{
    PLX_STAGE_SETUP = 1,
    PLX_STAGE_EXEC = 2,
    PLX_STAGE_UNKNOWN = 99 /* only sent by the PLX_UNKNOWN_STAGE test mode */
};

/* Child exit statuses. 127 means the child reached the report step and
   exited there; it is used even when send_record gave up partway, so a
   record may be absent or short. The test modes 126 and 125 use distinct
   statuses. */
enum
{
    PLX_CHILD_RECORD_SENT = 127,
    PLX_CHILD_EARLY_EXIT = 126,
    PLX_CHILD_EARLY_SIGNAL_SURVIVED = 125,
    PLX_CHILD_TREE_FAILED = 124
};

/* Largest errno value Linux can return; a record claiming more is corrupt. */
enum
{
    PLX_ERRNO_MAX = 4095
};

int plx_read_all(int fd, void *buffer, size_t length, size_t *used,
                 unsigned *interrupts)
{
    if (!used || !interrupts || length > PLX_MAX_TRANSFER_BYTES ||
        (!buffer && length))
    {
        return EINVAL;
    }
    *used = 0;
    *interrupts = 0;
    while (*used < length)
    {
        ssize_t n = read(fd, (unsigned char *)buffer + *used, length - *used);
        if (n > 0)
        {
            *used += (size_t)n;
        }
        else if (n == 0)
        {
            /* EOF before the promised length: the peer broke the protocol. */
            return EPROTO;
        }
        else if (errno == EINTR && PLX_MUTANT != 9)
        {
            ++*interrupts;
        }
        else
        {
            return errno;
        }
    }
    return 0;
}

int plx_write_all(int fd, const void *buffer, size_t length, size_t *used,
                  unsigned *interrupts)
{
    if (!used || !interrupts || length > PLX_MAX_TRANSFER_BYTES ||
        (!buffer && length))
    {
        return EINVAL;
    }
    *used = 0;
    *interrupts = 0;
    while (*used < length)
    {
        ssize_t n =
            write(fd, (const unsigned char *)buffer + *used, length - *used);
        if (n > 0)
        {
            *used += (size_t)n;
        }
        else if (n < 0 && errno == EINTR)
        {
            ++*interrupts;
        }
        else
        {
            /* write() returning 0 for a non-empty request makes no progress;
               report it as an I/O error rather than spinning. */
            return n == 0 ? EIO : errno;
        }
    }
    return 0;
}

int plx_wait(pid_t child, int *status, unsigned *interrupts)
{
    if (child <= 0 || !status || !interrupts)
    {
        return EINVAL;
    }
    *interrupts = 0;
    for (;;)
    {
        pid_t reaped = waitpid(child, status, 0);
        if (reaped == child)
        {
            return 0;
        }
        if (reaped < 0 && errno == EINTR && PLX_MUTANT != 10)
        {
            ++*interrupts;
            continue;
        }
        return reaped < 0 ? errno : EPROTO;
    }
}

static void store_le32(unsigned char *p, unsigned value)
{
    for (unsigned i = 0; i < PLX_FIELD_BYTES; ++i)
    {
        p[i] = (unsigned char)(value >> (8 * i));
    }
}

static uint32_t load_le32(const unsigned char *p)
{
    uint32_t value = 0;
    for (unsigned i = 0; i < PLX_FIELD_BYTES; ++i)
    {
        value |= (uint32_t)p[i] << (8 * i);
    }
    return value;
}

/* Bytes to move in one read()/write() call: at most `remaining`, and at most
   options.chunk when a chunk limit is set (0 means unlimited). The limit
   forces the short transfers the tests need to exercise. */
static size_t transfer_size(size_t remaining, unsigned chunk)
{
    if (chunk && remaining > chunk)
    {
        return chunk;
    }
    return remaining;
}

/* Child: writes `length` bytes of the record in chunk-sized pieces,
   retrying on EINTR and stopping at the first other failure. The parent
   judges completeness, not the child. */
static void send_record(int fd, const unsigned char *record, size_t length,
                        unsigned chunk)
{
    size_t sent = 0;
    while (sent < length)
    {
        size_t n = transfer_size(length - sent, chunk);
        ssize_t count = write(fd, record + sent, n);
        if (count > 0)
        {
            sent += (size_t)count;
            if (PLX_MUTANT == 1)
            {
                break;
            }
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

/* Child after fork(); never returns. The write end is O_CLOEXEC, so a
   successful execve() closes it and the parent reads EOF with no record.
   EOF is not proof that exec happened: the early-exit and early-signal
   modes also close the pipe without a record (no image proof), so the
   result is classified by the wait status. Only async-signal-safe calls are
   used before _exit(). */
static _Noreturn void run_child(const char *path, char *const args[],
                                struct plx_options options, int record_fd)
{
    if (options.mode == PLX_EARLY_EXIT)
    {
        _exit(PLX_CHILD_EARLY_EXIT);
    }
    if (options.mode == PLX_EARLY_SIGNAL)
    {
        kill(getpid(), SIGTERM);
        _exit(PLX_CHILD_EARLY_SIGNAL_SURVIVED);
    }
    unsigned stage = PLX_STAGE_SETUP;
    unsigned error = EACCES;
    if (options.mode == PLX_NORMAL)
    {
        char *const env[] = {NULL};
        execve(path, args, env);
        stage = PLX_STAGE_EXEC;
        error = (unsigned)errno;
    }
    if (options.mode == PLX_UNKNOWN_STAGE)
    {
        stage = PLX_STAGE_UNKNOWN;
    }
    unsigned char record[PLX_RECORD_BYTES];
    store_le32(record, stage);
    store_le32(record + PLX_FIELD_BYTES, error);
    /* PLX_SHORT_RECORD sends a truncated record on purpose. */
    size_t length = options.mode == PLX_SHORT_RECORD ? 3 : PLX_RECORD_BYTES;
    send_record(record_fd, record, length, options.chunk);
    _exit(PLX_CHILD_RECORD_SENT);
}

/* Parent: reads until EOF or until the buffer is full. The buffer is one
   byte larger than a valid record so that an over-long record is noticed as
   used > PLX_RECORD_BYTES instead of being silently truncated. Returns 0 or
   the errno of a read failure; *used is the byte count received. */
static int read_record(int fd, unsigned char *record, size_t capacity,
                       unsigned chunk, size_t *used)
{
    *used = 0;
    while (*used < capacity)
    {
        size_t n = transfer_size(capacity - *used, chunk);
        ssize_t count = read(fd, record + *used, n);
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

/* Validates a complete record against the exit status and builds the
   PLX_FAILED result. The child must have exited with exactly the
   record-sent status: a record from a process that was killed, or that
   exited otherwise, cannot be trusted. Returns 0 or EPROTO. */
static int decode_failure(const unsigned char *record, int status,
                          struct plx_result *result)
{
    uint32_t stage = load_le32(record);
    uint32_t code = load_le32(record + PLX_FIELD_BYTES);
    if ((stage != PLX_STAGE_SETUP && stage != PLX_STAGE_EXEC) &&
        PLX_MUTANT != 3)
    {
        return EPROTO;
    }
    if (code == 0 || code > PLX_ERRNO_MAX || !WIFEXITED(status) ||
        WEXITSTATUS(status) != PLX_CHILD_RECORD_SENT)
    {
        return EPROTO;
    }
    *result = (struct plx_result){PLX_FAILED, (int)code, (int)stage};
    return 0;
}

/* Contract: see launch.h. The parent owns both pipe ends until return and
   closes them on every path. A forked child is reaped (plx_wait) before any
   result is judged, unless plx_wait fails with an error other than EINTR;
   that error is returned and the child stays unreaped (hard wait failure is
   outside the contract). On failure *out is untouched. Blocks until the
   child ends. */
int plx_launch(const char *path, char *const args[], struct plx_options options,
               struct plx_result *out)
{
    if (!path || !args || !args[0] || !out || options.mode > PLX_EARLY_SIGNAL ||
        options.chunk > PLX_RECORD_BYTES)
    {
        return EINVAL;
    }
    int pipefd[2];
    if (pipe2(pipefd, O_CLOEXEC) < 0)
    {
        return errno;
    }
    pid_t child = fork();
    if (child < 0)
    {
        int error = errno;
        close(pipefd[0]);
        close(pipefd[1]);
        return error;
    }
    if (child == 0)
    {
        close(pipefd[0]);
        run_child(path, args, options, pipefd[1]);
    }
    /* Parent: the write end must be closed here or EOF never arrives. */
    close(pipefd[1]);
    unsigned char record[PLX_RECORD_BYTES + 1];
    size_t used;
    int read_error =
        read_record(pipefd[0], record, sizeof record, options.chunk, &used);
    close(pipefd[0]);

    int status;
    unsigned interruptions;
    int wait_error = plx_wait(child, &status, &interruptions);
    if (wait_error)
    {
        return wait_error;
    }
    if (read_error)
    {
        return read_error;
    }
    if (used && used != PLX_RECORD_BYTES)
    {
        if (PLX_MUTANT != 2)
        {
            return EPROTO;
        }
        used = 0;
    }
    struct plx_result result = {0, 0, 0};
    if (used)
    {
        int protocol_error = decode_failure(record, status, &result);
        if (protocol_error)
        {
            return protocol_error;
        }
    }
    else if (WIFEXITED(status))
    {
        result = (struct plx_result){PLX_EXITED, WEXITSTATUS(status), 0};
    }
    else if (WIFSIGNALED(status))
    {
        result = (struct plx_result){PLX_SIGNALED, WTERMSIG(status), 0};
    }
    else
    {
        return EPROTO;
    }
    if (PLX_MUTANT == 4 && !used)
    {
        result = (struct plx_result){PLX_EXITED, 0, 0};
    }
    *out = result;
    return 0;
}

/* Phase 1 of plx_redirect(): duplicate both sources to descriptors numbered
   above every destination. Because no copy can occupy a destination number,
   installing destination 0 cannot destroy the copy that destination 1 still
   needs, even when the pairs cross (source 0 == destination 1). The `fail`
   values 1 and 2 inject a failure before copy 0 or copy 1. Returns 0 or an
   errno; copies that exist on return are owned by the caller. */
static int copy_sources(const int sources[2], int minimum, unsigned fail,
                        int copies[2])
{
    for (unsigned i = 0; i < 2; ++i)
    {
        if (fail == i + 1)
        {
            return EIO;
        }
        copies[i] = fcntl(sources[i], F_DUPFD_CLOEXEC, minimum);
        if (copies[i] < 0)
        {
            return errno;
        }
    }
    return 0;
}

/* Phase 2: install the copies on the destinations. dup2() clears
   FD_CLOEXEC on the destination, which is the point of redirecting before
   exec. The `fail` values 3 and 4 inject a failure before install 0 or 1.
   Returns 0 or an errno. */
static int install_copies(const int copies[2], const int sources[2],
                          const int destinations[2], unsigned fail)
{
    for (unsigned i = 0; i < 2; ++i)
    {
        if (fail == i + 3)
        {
            return EIO;
        }
        if (PLX_MUTANT == 6 && sources[i] == destinations[i])
        {
            continue;
        }
        while (dup2(copies[i], destinations[i]) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return errno;
        }
    }
    return 0;
}

/* Contract: see launch.h. All descriptors are borrowed. On every return the
   temporary copies are closed, so the descriptor table differs from the
   entry state only by the installed destinations. */
int plx_redirect(const int sources[2], const int destinations[2], unsigned fail)
{
    if (!sources || !destinations || fail > 4 || sources[0] < 0 ||
        sources[1] < 0 || destinations[0] < 0 || destinations[1] < 0 ||
        destinations[0] == destinations[1])
    {
        return EINVAL;
    }
    int largest =
        destinations[0] > destinations[1] ? destinations[0] : destinations[1];
    if (largest == INT_MAX)
    {
        return EINVAL;
    }
    if (PLX_MUTANT == 5)
    {
        /* Naive in-place dup2: breaks when the pairs cross. */
        for (unsigned i = 0; i < 2; ++i)
        {
            if (dup2(sources[i], destinations[i]) < 0)
            {
                return errno;
            }
        }
        return 0;
    }
    int copies[2] = {-1, -1};
    int error = copy_sources(sources, largest + 1, fail, copies);
    if (!error)
    {
        error = install_copies(copies, sources, destinations, fail);
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        if (copies[i] >= 0)
        {
            close(copies[i]);
        }
    }
    return error;
}

/* Child of plx_tree(): holds `writer` only until `repair` closes it early.
   Signals readiness, waits for the release byte, then exits. If the writer
   stayed open, a reader of the data pipe cannot see EOF; that inherited
   write end is the defect the repair fixes. Never returns. */
static _Noreturn void run_tree_child(int writer, int release, int ready,
                                     int repair)
{
    int writer_closed = 0;
    if ((repair && PLX_MUTANT != 7) || (!repair && PLX_MUTANT == 8))
    {
        close(writer);
        writer_closed = 1;
    }
    char token = 'R';
    size_t used;
    unsigned interruptions;
    int error = plx_write_all(ready, &token, 1, &used, &interruptions);
    close(ready);
    if (!error)
    {
        error = plx_read_all(release, &token, 1, &used, &interruptions);
    }
    close(release);
    if (!writer_closed)
    {
        close(writer);
    }
    _exit(error ? PLX_CHILD_TREE_FAILED : 0);
}

int plx_tree(int writer, int release, int ready, int repair)
{
    if (writer < 0 || release < 0 || ready < 0 || (repair != 0 && repair != 1))
    {
        return EINVAL;
    }
    pid_t child = fork();
    if (child < 0)
    {
        int error = errno;
        close(writer);
        close(release);
        close(ready);
        return error;
    }
    if (child == 0)
    {
        run_tree_child(writer, release, ready, repair);
    }
    close(writer);
    close(release);
    close(ready);
    int status;
    unsigned interruptions;
    int error = plx_wait(child, &status, &interruptions);
    if (error)
    {
        return error;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : EPROTO;
}
