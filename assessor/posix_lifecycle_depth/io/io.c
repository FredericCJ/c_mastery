#define _POSIX_C_SOURCE 200809L
#include "io.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#ifndef PC_MUTANT
#define PC_MUTANT 0
#endif

/* pc_write_record() never asks the kernel for more than this many bytes in
   one pwrite() call. The value is a choice of this implementation, not a
   requirement of the contract. */
enum
{
    PC_WRITE_CHUNK_BYTES = 4096
};

void pc_init(struct pc_copy *copy)
{
    if (copy)
    {
        memset(copy, 0, sizeof *copy);
    }
}

/* Latches `error` in the state machine: every later pc_resume() call
   returns PC_ERROR without touching the callbacks. */
static enum pc_status fail(struct pc_copy *copy, int error)
{
    copy->error = error;
    return PC_ERROR;
}

static bool would_block(int error)
{
    return error == EAGAIN || error == EWOULDBLOCK;
}

/* A callback that fails without setting errno is still a failure. */
static int errno_or_eio(int error)
{
    return error ? error : EIO;
}

/* Writes the pending bytes [next, end) once.
 *
 * Returns true when pc_resume() must return *status now, false when it
 * should spend its next budget unit (progress or a retried EINTR).
 * `committed` counts bytes the sink acknowledged, never bytes offered, so an
 * overflow check is made before the call and a count above the request is a
 * protocol error rather than a silent over-commit. */
static bool write_pending(struct pc_copy *copy, struct pc_ops ops,
                          enum pc_status *status)
{
    size_t count = copy->end - copy->next;
    if (copy->committed > UINTMAX_MAX - count)
    {
        *status = fail(copy, EOVERFLOW);
        return true;
    }
    ssize_t written = ops.write(ops.context, copy->bytes + copy->next, count);
    if (written < 0)
    {
        int error = errno;
        if (error == EINTR)
        {
            return false;
        }
        if (would_block(error))
        {
#if PC_MUTANT == 1
            copy->next = 0;
#endif
            *status = PC_WAIT_WRITE;
            return true;
        }
        *status = fail(copy, errno_or_eio(error));
        return true;
    }
    if (written == 0)
    {
        *status = fail(copy, EIO);
        return true;
    }
    if ((size_t)written > count)
    {
        *status = fail(copy, EPROTO);
        return true;
    }
    copy->committed += (size_t)written;
#if PC_MUTANT == 2
    copy->next = copy->end;
#else
    copy->next += (size_t)written;
#endif
    return false;
}

/* Refills the empty buffer from the source. Same return convention as
 * write_pending(). Zero bytes is end of input: it is latched in copy->eof
 * so that a later call finishes without reading again. */
static bool refill_buffer(struct pc_copy *copy, struct pc_ops ops,
                          enum pc_status *status)
{
    if (copy->eof)
    {
        *status = PC_DONE;
        return true;
    }
    copy->next = 0;
    copy->end = 0;
    ssize_t received = ops.read(ops.context, copy->bytes, sizeof copy->bytes);
    if (received < 0)
    {
        int error = errno;
        if (error == EINTR)
        {
            return false;
        }
        if (would_block(error))
        {
            *status = PC_WAIT_READ;
            return true;
        }
        *status = fail(copy, errno_or_eio(error));
        return true;
    }
    if ((size_t)received > sizeof copy->bytes)
    {
        *status = fail(copy, EPROTO);
        return true;
    }
    if (received == 0)
    {
        copy->eof = true;
        *status = PC_DONE;
        return true;
    }
    copy->end = (size_t)received;
    return false;
}

/* Contract: one call performs at most `budget` callback invocations (an
 * EINTR retry also costs one), then returns PC_YIELD so a cooperative caller
 * can interleave other work. State is kept entirely in *copy, so the copy
 * can be resumed after PC_WAIT_READ/PC_WAIT_WRITE once the caller has waited
 * for the descriptor. Invariant between calls: next <= end <= sizeof bytes.
 * A latched error stays PC_ERROR. */
enum pc_status pc_resume(struct pc_copy *copy, struct pc_ops ops,
                         unsigned budget)
{
    if (!copy || !ops.read || !ops.write)
    {
        return PC_ERROR;
    }
    if (copy->error)
    {
        return PC_ERROR;
    }
    if (copy->next > copy->end || copy->end > sizeof copy->bytes)
    {
        return fail(copy, EINVAL);
    }
    while (budget--)
    {
        enum pc_status status = PC_YIELD;
        bool finished;
        if (copy->next < copy->end)
        {
            finished = write_pending(copy, ops, &status);
        }
        else
        {
            finished = refill_buffer(copy, ops, &status);
        }
        if (finished)
        {
            return status;
        }
    }
    return copy->eof && copy->next == copy->end ? PC_DONE : PC_YIELD;
}

uintmax_t pc_off_max(void)
{
    _Static_assert((off_t)-1 < 0,
                   "This Linux ABI contract requires signed off_t");
    _Static_assert(sizeof(off_t) <= sizeof(uintmax_t),
                   "off_t wider than course conversion");
    /* Linux selected ABI: no padding bits, conventional signed off_t. The
       maximum is a mask of all value bits: shift out the sign bit and any
       width difference from uintmax_t. */
    return UINTMAX_MAX >>
           (sizeof(uintmax_t) * CHAR_BIT - sizeof(off_t) * CHAR_BIT + 1);
}

/* Checks that [offset, offset + length) fits both the off_t range and the
   current file size, without computing offset + length (which could wrap).
   Returns EOVERFLOW for off_t, ERANGE for the file, else 0 and *converted. */
int pc_extent(uintmax_t offset, uintmax_t length, uintmax_t file_size,
              off_t *converted)
{
    if (!converted)
    {
        return EINVAL;
    }
#if PC_MUTANT != 3
    if (offset > pc_off_max() || length > pc_off_max() - offset)
    {
        return EOVERFLOW;
    }
#endif
#if PC_MUTANT != 4
    if (offset > file_size || length > file_size - offset)
    {
        return ERANGE;
    }
#else
    (void)file_size;
#endif
    *converted = (off_t)offset;
    return 0;
}

/* pwrite() ignores its offset when the descriptor is O_APPEND on Linux, so
   a positioned record write on such a descriptor would land at end of file.
   Returns 0 when the descriptor is usable, else an errno. */
static int check_positioned_write_allowed(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0)
    {
        return errno;
    }
#if PC_MUTANT != 5
    if (flags & O_APPEND)
    {
        return EINVAL;
    }
#endif
    return 0;
}

int pc_write_record(int fd, uintmax_t offset, const unsigned char *data,
                    size_t length, uintmax_t file_size, size_t *committed)
{
    if (!committed || (!data && length))
    {
        return EINVAL;
    }
    off_t where = 0;
    int error = pc_extent(offset, length, file_size, &where);
    if (error)
    {
        return error;
    }
    error = check_positioned_write_allowed(fd);
    if (error)
    {
        return error;
    }
    /* From here *committed is always the exact prefix written, including on
       failure, so a caller can resume or report precisely. */
    *committed = 0;
    size_t done = 0;
    while (done < length)
    {
        size_t chunk = length - done;
        if (chunk > PC_WRITE_CHUNK_BYTES)
        {
            chunk = PC_WRITE_CHUNK_BYTES;
        }
#if PC_MUTANT == 6
        ssize_t n = write(fd, data + done, chunk);
#else
        ssize_t n = pwrite(fd, data + done, chunk, where + (off_t)done);
#endif
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            *committed = done;
            return errno;
        }
        if (n == 0)
        {
            *committed = done;
            return EIO;
        }
        done += (size_t)n;
    }
    *committed = done;
    return 0;
}

int pc_close_transition(bool *owned, int error, enum pc_close_policy policy)
{
    if (!owned || !*owned ||
        (policy != PC_LINUX_RELEASES && policy != PC_EINTR_RETAINS) ||
        error < 0)
    {
        return EINVAL;
    }
#if PC_MUTANT == 7
    policy = PC_EINTR_RETAINS;
#elif PC_MUTANT == 8
    policy = PC_LINUX_RELEASES;
#endif
    /* Linux releases the descriptor even when close() reports EINTR;
       retrying would close an unrelated descriptor another thread may have
       just been given. Under the retaining policy only EINTR keeps
       ownership. */
    if (error != EINTR || policy == PC_LINUX_RELEASES)
    {
        *owned = false;
    }
    return error;
}
