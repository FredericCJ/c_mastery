#define _GNU_SOURCE
#include "map.h"
#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#ifndef PM_MUTANT
#define PM_MUTANT 0
#endif

/* A mapping may cover at most this many pages. */
enum
{
    PM_MAX_MAP_PAGES = 16
};

/* Wire layout of a versioned record (128 bytes, all integers in the
   endianness named by the endian byte):

     0..3    magic "PMR1"
     4       state: 1 = stable, 0 = a publisher is mid-update (reader retries)
     5       format version, 1
     6       endian: 1 = little, 2 = big
     7       reserved, zero
     8..15   session identifier, nonzero
     16..19  payload offset, 32
     20..23  payload length, at most 64
     24..27  checksum of the payload
     28..31  reserved, zero
     32..    payload, zero padded to 128 bytes

   Every field offset is spelled once, here. */
enum
{
    PM_RECORD_BYTES = 128,
    PM_HEADER_BYTES = 32,
    PM_PAYLOAD_MAX_BYTES = 64,
    PM_MAGIC_BYTES = 4,
    PM_OFF_STATE = 4,
    PM_OFF_VERSION = 5,
    PM_OFF_ENDIAN = 6,
    PM_OFF_RESERVED0 = 7,
    PM_OFF_SESSION = 8,
    PM_OFF_PAYLOAD_OFFSET = 16,
    PM_OFF_PAYLOAD_LENGTH = 20,
    PM_OFF_CHECKSUM = 24,
    PM_OFF_RESERVED1 = 28,
    PM_RESERVED1_BYTES = 4,
    PM_SESSION_BYTES = 8,
    PM_WORD_BYTES = 4,
    PM_STATE_BUSY = 0,
    PM_STATE_STABLE = 1,
    PM_FORMAT_VERSION = 1,
    PM_ENDIAN_LITTLE = 1,
    PM_ENDIAN_BIG = 2
};

static const char pm_magic[PM_MAGIC_BYTES] = {'P', 'M', 'R', '1'};

/* Contract: see map.h. A successful call returns an active read-only shared
   mapping that the caller must release with pm_unmap(); the descriptor stays
   borrowed. On failure *out is untouched and nothing is mapped. */
int pm_map(int fd, off_t offset, size_t mapped, size_t logical,
           struct pm_map *out)
{
    long page = sysconf(_SC_PAGESIZE);
    if (!out || page <= 0 || offset < 0 || offset % page || !mapped ||
        logical > mapped ||
        (uintmax_t)mapped > PM_MAX_MAP_PAGES * (uintmax_t)page)
    {
        return EINVAL;
    }
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        return errno;
    }
    /* The logical window must lie inside the file; the mapped extent may
       exceed it (the tail past end of file is what causes SIGBUS). */
    if (st.st_size < 0 || (uintmax_t)offset > (uintmax_t)st.st_size ||
        (uintmax_t)logical > (uintmax_t)st.st_size - (uintmax_t)offset)
    {
        return ERANGE;
    }
    void *base = mmap(NULL, mapped, PROT_READ, MAP_SHARED, fd, offset);
    if (base == MAP_FAILED)
    {
        return errno;
    }
    *out = (struct pm_map){.base = base,
                           .mapped = mapped,
                           .logical = logical,
                           .offset = offset,
                           .device = st.st_dev,
                           .inode = st.st_ino,
                           .generation = 1,
                           .active = 1};
    return 0;
}

/* Replaces the mapping with a new one over the same file object and bumps
   the generation, which invalidates every outstanding struct pm_borrow.
   The new mapping is created before the old one is removed, so a failure
   leaves *map exactly as it was. */
int pm_remap(struct pm_map *map, int fd, size_t mapped, size_t logical)
{
    if (!map || !map->active)
    {
        return EINVAL;
    }
    if (map->generation == UINT64_MAX)
    {
        return EOVERFLOW;
    }
    struct pm_map next;
    int error = pm_map(fd, map->offset, mapped, logical, &next);
    if (error)
    {
        return error;
    }
    if (next.device != map->device || next.inode != map->inode)
    {
        pm_unmap(&next);
        return EXDEV;
    }
    if (munmap(map->base, map->mapped) < 0)
    {
        error = errno;
        pm_unmap(&next);
        return error;
    }
    next.generation = map->generation + (PM_MUTANT == 2 ? 0 : 1);
    *map = next;
    return 0;
}

int pm_copy(const struct pm_map *map, struct pm_borrow borrow, void *out,
            size_t capacity)
{
    if (!map || !map->active || (!out && borrow.length))
    {
        return EINVAL;
    }
    if (borrow.generation != map->generation)
    {
        return ESTALE;
    }
    /* Reads are limited to the logical window, not the whole mapping. */
    size_t limit = PM_MUTANT == 1 ? map->mapped : map->logical;
    if (borrow.offset > limit || borrow.length > limit - borrow.offset ||
        borrow.length > capacity)
    {
        return ERANGE;
    }
    if (borrow.length)
    {
        memcpy(out, map->base + borrow.offset, borrow.length);
    }
    return 0;
}

int pm_unmap(struct pm_map *map)
{
    if (!map)
    {
        return EINVAL;
    }
    if (!map->active)
    {
        return 0;
    }
    if (munmap(map->base, map->mapped) < 0)
    {
        return errno;
    }
    map->base = NULL;
    map->mapped = 0;
    map->logical = 0;
    map->active = 0;
    return 0;
}

/* Byte index of significance `i` (0 = least significant) in a field of
   `count` bytes stored with the given endianness. */
static unsigned field_index(unsigned i, unsigned count, unsigned endian)
{
    return endian == PM_ENDIAN_LITTLE ? i : count - 1 - i;
}

static void store_uint(unsigned char *p, uint64_t value, unsigned count,
                       unsigned endian)
{
    for (unsigned i = 0; i < count; ++i)
    {
        p[field_index(i, count, endian)] = (unsigned char)(value >> (8 * i));
    }
}

static uint64_t load_uint(const unsigned char *p, unsigned count,
                          unsigned endian)
{
    uint64_t value = 0;
    for (unsigned i = 0; i < count; ++i)
    {
        value |= (uint64_t)p[field_index(i, count, endian)] << (8 * i);
    }
    return value;
}

/* Multiplicative checksum (times 33, wrapping in 32 bits). It detects
   accidental corruption and torn writes in the teaching format; it is not
   a cryptographic integrity check. */
static uint32_t payload_checksum(const unsigned char *p, size_t n)
{
    uint32_t value = 0;
    for (size_t i = 0; i < n; ++i)
    {
        value = value * 33u + p[i];
    }
    return value;
}

int pm_encode(unsigned char bytes[PM_RECORD_BYTES],
              const struct pm_record *record, unsigned endian)
{
    if (!bytes || !record || !record->session ||
        record->length > PM_PAYLOAD_MAX_BYTES ||
        (endian != PM_ENDIAN_LITTLE && endian != PM_ENDIAN_BIG))
    {
        return EINVAL;
    }
    /* Built in a scratch buffer so *bytes is never half written. */
    unsigned char temp[PM_RECORD_BYTES] = {0};
    memcpy(temp, pm_magic, PM_MAGIC_BYTES);
    temp[PM_OFF_STATE] = PM_STATE_STABLE;
    temp[PM_OFF_VERSION] = PM_FORMAT_VERSION;
    temp[PM_OFF_ENDIAN] = (unsigned char)endian;
    store_uint(temp + PM_OFF_SESSION, record->session, PM_SESSION_BYTES,
               endian);
    store_uint(temp + PM_OFF_PAYLOAD_OFFSET, PM_HEADER_BYTES, PM_WORD_BYTES,
               endian);
    store_uint(temp + PM_OFF_PAYLOAD_LENGTH, record->length, PM_WORD_BYTES,
               endian);
    memcpy(temp + PM_HEADER_BYTES, record->payload, record->length);
    store_uint(temp + PM_OFF_CHECKSUM,
               payload_checksum(temp + PM_HEADER_BYTES, record->length),
               PM_WORD_BYTES, endian);
    memcpy(bytes, temp, PM_RECORD_BYTES);
    return 0;
}

static bool bytes_are_zero(const unsigned char *p, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        if (p[i] != 0)
        {
            return false;
        }
    }
    return true;
}

/* The header bytes that must be exactly as pm_encode() wrote them. */
static bool header_is_well_formed(const unsigned char *bytes, unsigned endian)
{
    return memcmp(bytes, pm_magic, PM_MAGIC_BYTES) == 0 &&
           bytes[PM_OFF_VERSION] == PM_FORMAT_VERSION &&
           (endian == PM_ENDIAN_LITTLE || endian == PM_ENDIAN_BIG) &&
           bytes[PM_OFF_RESERVED0] == 0 &&
           bytes_are_zero(bytes + PM_OFF_RESERVED1, PM_RESERVED1_BYTES);
}

/* Returns 0 and fills *out for a stable valid record, EAGAIN while a
   publisher holds the record mid-update (the caller may retry), EINVAL for
   null arguments and EPROTO for anything malformed. The input is never
   trusted: every length and offset is range checked against `extent`
   before use. */
int pm_decode(const unsigned char *bytes, size_t extent, struct pm_record *out)
{
    if (!bytes || !out)
    {
        return EINVAL;
    }
    if (extent != PM_RECORD_BYTES)
    {
        return EPROTO;
    }
    if (bytes[PM_OFF_STATE] == PM_STATE_BUSY && PM_MUTANT != 3)
    {
        return EAGAIN;
    }
    if (bytes[PM_OFF_STATE] > PM_STATE_STABLE)
    {
        return EPROTO;
    }
    unsigned endian = PM_MUTANT == 4 ? PM_ENDIAN_LITTLE : bytes[PM_OFF_ENDIAN];
    if (!header_is_well_formed(bytes, endian))
    {
        return EPROTO;
    }
    uint64_t session =
        load_uint(bytes + PM_OFF_SESSION, PM_SESSION_BYTES, endian);
    uint64_t offset =
        load_uint(bytes + PM_OFF_PAYLOAD_OFFSET, PM_WORD_BYTES, endian);
    uint64_t length =
        load_uint(bytes + PM_OFF_PAYLOAD_LENGTH, PM_WORD_BYTES, endian);
    if (!session || offset < PM_HEADER_BYTES || offset > extent ||
        length > extent - offset || length > PM_PAYLOAD_MAX_BYTES)
    {
        return EPROTO;
    }
    uint64_t stored_checksum =
        load_uint(bytes + PM_OFF_CHECKSUM, PM_WORD_BYTES, endian);
    if (stored_checksum !=
        payload_checksum(bytes + (size_t)offset, (size_t)length))
    {
        return EPROTO;
    }
    struct pm_record result = {session, (size_t)length, {0}};
    memcpy(result.payload, bytes + (size_t)offset, (size_t)length);
    *out = result;
    return 0;
}

/* Whole-file advisory record lock through fcntl(). These locks belong to the
   process, not the descriptor, and are dropped when the process dies, which
   is what makes owner-death recovery possible. Returns 0 or an errno. */
static int lock_record(int fd, short type, int blocking)
{
    struct flock lock = {0};
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0; /* 0 means "to end of file", however it grows */
    for (;;)
    {
        if (fcntl(fd, blocking ? F_SETLKW : F_SETLK, &lock) == 0)
        {
            return 0;
        }
        if (errno == EINTR)
        {
            continue;
        }
        return errno;
    }
}

/* Maps the record file read-write. On success the caller owns *bytes and
   releases it with munmap(*bytes, PM_RECORD_BYTES). */
static int map_record(int fd, unsigned char **bytes)
{
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        return errno;
    }
    if (st.st_size != PM_RECORD_BYTES)
    {
        return EPROTO;
    }
    void *base =
        mmap(NULL, PM_RECORD_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED)
    {
        return errno;
    }
    *bytes = base;
    return 0;
}

/* A fresh file is empty: size it so that it can be mapped. Anything other
   than empty or exactly one record is not ours. */
static int ensure_record_size(int fd)
{
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        return errno;
    }
    if (st.st_size == 0)
    {
        if (ftruncate(fd, PM_RECORD_BYTES) < 0)
        {
            return errno;
        }
        return 0;
    }
    if (st.st_size != PM_RECORD_BYTES)
    {
        return EPROTO;
    }
    return 0;
}

/* Decides whether the record currently in the file may be overwritten.
   Publishing (replace == 0): a valid record is EEXIST; an empty or busy
   record (decode result EAGAIN) is taken over and returns 0; any other
   decode failure (EPROTO) is returned as is. Replacing (replace != 0): a
   valid record with session `expected` returns 0 and a valid record with
   another session is ESTALE; an empty or busy record is EAGAIN, and a
   malformed record is EPROTO. Returns 0 to go on. */
static int check_overwrite_allowed(const unsigned char *bytes, int replace,
                                   uint64_t expected)
{
    struct pm_record old;
    int decoded = pm_decode(bytes, PM_RECORD_BYTES, &old);
    if (!decoded)
    {
        if (!replace)
        {
            return EEXIST;
        }
        return old.session == expected ? 0 : ESTALE;
    }
    if (replace)
    {
        return decoded;
    }
    /* EAGAIN means an earlier publisher died mid-update: take over. */
    return decoded == EAGAIN ? 0 : decoded;
}

/* Update protocol, in this order, inside the exclusive record lock:
   (1) mark the record busy so that a lock-free reader sees EAGAIN;
   (2) write everything except the state byte;
   (3) mark it stable last.
   Property: whenever the publisher is killed, the bytes left in the file
   are either a stable record whose body is complete, or a record marked
   busy (state byte 0); a stable state byte never covers a torn body.
   `cut`, when given, observes (and the tests may kill the process at)
   each numbered point.

   How the code guarantees the order: the three steps are plain stores to
   a shared mapping, and ISO C lets an optimizer move them relative to
   each other (gcc 15 -O2 did store the stable byte before the body copy
   when `cut` is NULL). Each step is therefore separated from the next by
   atomic_signal_fence(memory_order_seq_cst), a compiler barrier that
   forbids moving memory accesses across it.

   Why a signal fence and not atomic_thread_fence(memory_order_release):
   the property is about a single writer thread that is killed, and about
   a successor that looks only after the kill. A process killed at an
   instruction boundary leaves every store it executed before that point
   in the page cache, so the file shows a prefix of the program order;
   the only thing that can break the prefix is the compiler. No other
   process reads the record while the writer is alive: pm_read_record
   needs the shared record lock, which the writer holds exclusively, and
   lock and unlock are system calls that already order memory between
   processes (CONTRACTS.md claims no lock-free concurrent observer and no
   portable interprocess atomics). A release fence would add a CPU
   barrier for an observer that does not exist, and it would still be
   incomplete: the reader would need a matching acquire and the state
   byte would have to be an atomic object, or the unlocked access would
   be a data race in ISO C. A lock-free reader would need exactly that
   change (atomic state byte, release store, acquire load); this
   reference does not provide it. Power loss or a machine crash is not
   covered, since that would need msync and ordering of the writeback. */
static void write_record_bytes(unsigned char *bytes,
                               const unsigned char fresh[PM_RECORD_BYTES],
                               pm_cut_fn cut, void *context)
{
    /* Invalidate: nothing read or written earlier may move below this. */
    atomic_signal_fence(memory_order_seq_cst);
    bytes[PM_OFF_STATE] = PM_STATE_BUSY;
    if (cut)
    {
        cut(1, context);
    }
    /* The busy mark must be stored before any body byte. */
    atomic_signal_fence(memory_order_seq_cst);
    memcpy(bytes, fresh, PM_OFF_STATE);
    memcpy(bytes + PM_OFF_VERSION, fresh + PM_OFF_VERSION,
           PM_RECORD_BYTES - PM_OFF_VERSION);
    if (cut)
    {
        cut(2, context);
    }
    /* Publish: every body byte must be stored before the stable byte. */
    atomic_signal_fence(memory_order_seq_cst);
    bytes[PM_OFF_STATE] = PM_STATE_STABLE;
    /* The stable mark must not sink below the cut-3 observation. */
    atomic_signal_fence(memory_order_seq_cst);
    if (cut)
    {
        cut(3, context);
    }
}

/* Caller holds the exclusive record lock. */
static int publish_locked(int fd, uint64_t session, const void *data,
                          size_t length, pm_cut_fn cut, void *context,
                          int replace, uint64_t expected)
{
    int error = ensure_record_size(fd);
    if (error)
    {
        return error;
    }
    unsigned char *bytes = NULL;
    error = map_record(fd, &bytes);
    if (error)
    {
        return error;
    }
    error = check_overwrite_allowed(bytes, replace, expected);
    if (!error)
    {
        struct pm_record record = {session, length, {0}};
        if (length)
        {
            memcpy(record.payload, data, length);
        }
        unsigned char fresh[PM_RECORD_BYTES];
        error = pm_encode(fresh, &record, PM_ENDIAN_LITTLE);
        if (!error)
        {
            write_record_bytes(bytes, fresh, cut, context);
        }
    }
    /* A failure reported above wins; an unmap failure matters only after a
       successful publish. */
    int unmap_error = munmap(bytes, PM_RECORD_BYTES) == 0 ? 0 : errno;
    return error ? error : unmap_error;
}

int pm_publish(int fd, uint64_t session, const void *data, size_t length,
               pm_cut_fn cut, void *context)
{
    if (!session || length > PM_PAYLOAD_MAX_BYTES || (!data && length))
    {
        return EINVAL;
    }
    int error = lock_record(fd, F_WRLCK, 1);
    if (error)
    {
        return error;
    }
    error = publish_locked(fd, session, data, length, cut, context, 0, 0);
    int unlock = lock_record(fd, F_UNLCK, 0);
    return error ? error : unlock;
}

/* Reads under a shared lock (non-blocking: a busy writer is reported, not
   waited for) and returns the record only if its session is `expected`. */
int pm_read_record(int fd, uint64_t expected, struct pm_record *out)
{
    if (!expected || !out)
    {
        return EINVAL;
    }
    int error = PM_MUTANT == 5 ? 0 : lock_record(fd, F_RDLCK, 0);
    if (error)
    {
        return error;
    }
    unsigned char *bytes = NULL;
    struct pm_record result = {0};
    error = map_record(fd, &bytes);
    if (!error)
    {
        error = pm_decode(bytes, PM_RECORD_BYTES, &result);
        int unmapped = munmap(bytes, PM_RECORD_BYTES);
        if (!error && unmapped < 0)
        {
            error = errno;
        }
    }
    int unlock = PM_MUTANT == 5 ? 0 : lock_record(fd, F_UNLCK, 0);
    if (!error)
    {
        error = unlock;
    }
    if (!error && result.session != expected && PM_MUTANT != 6)
    {
        error = ESTALE;
    }
    if (!error)
    {
        *out = result;
    }
    return error;
}

int pm_replace(int fd, uint64_t expected, uint64_t replacement,
               const void *data, size_t length)
{
    if (!expected || replacement <= expected || length > PM_PAYLOAD_MAX_BYTES ||
        (!data && length))
    {
        return EINVAL;
    }
    int error = lock_record(fd, F_WRLCK, 1);
    if (error)
    {
        return error;
    }
    error =
        publish_locked(fd, replacement, data, length, NULL, NULL, 1, expected);
    int unlock = lock_record(fd, F_UNLCK, 0);
    return error ? error : unlock;
}
