#define _GNU_SOURCE
#include "map.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#ifndef PM_MUTANT
#define PM_MUTANT 0
#endif
int pm_map(int fd, off_t offset, size_t mapped, size_t logical,
           struct pm_map *out)
{
    long page = sysconf(_SC_PAGESIZE);
    struct stat st;
    if (!out || page <= 0 || offset < 0 || offset % page || !mapped ||
        logical > mapped || (uintmax_t)mapped > 16u * (uintmax_t)page)
        return EINVAL;
    if (fstat(fd, &st) < 0)
        return errno;
    if (st.st_size < 0 || (uintmax_t)offset > (uintmax_t)st.st_size ||
        (uintmax_t)logical > (uintmax_t)st.st_size - (uintmax_t)offset)
        return ERANGE;
    void *base = mmap(NULL, mapped, PROT_READ, MAP_SHARED, fd, offset);
    if (base == MAP_FAILED)
        return errno;
    *out = (struct pm_map){base,      mapped,    logical, offset,
                           st.st_dev, st.st_ino, 1,       1};
    return 0;
}

int pm_remap(struct pm_map *map, int fd, size_t mapped, size_t logical)
{
    if (!map || !map->active)
        return EINVAL;
    if (map->generation == UINT64_MAX)
        return EOVERFLOW;
    struct pm_map next;
    int e = pm_map(fd, map->offset, mapped, logical, &next);
    if (e)
        return e;
    if (next.device != map->device || next.inode != map->inode)
    {
        pm_unmap(&next);
        return EXDEV;
    }
    if (munmap(map->base, map->mapped) < 0)
    {
        e = errno;
        pm_unmap(&next);
        return e;
    }
    next.generation = map->generation + (PM_MUTANT == 2 ? 0 : 1);
    *map = next;
    return 0;
}

int pm_copy(const struct pm_map *map, struct pm_borrow borrow, void *out,
            size_t capacity)
{
    if (!map || !map->active || (!out && borrow.length))
        return EINVAL;
    if (borrow.generation != map->generation)
        return ESTALE;
    size_t limit = PM_MUTANT == 1 ? map->mapped : map->logical;
    if (borrow.offset > limit || borrow.length > limit - borrow.offset ||
        borrow.length > capacity)
        return ERANGE;
    if (borrow.length)
        memcpy(out, map->base + borrow.offset, borrow.length);
    return 0;
}

int pm_unmap(struct pm_map *map)
{
    if (!map)
        return EINVAL;
    if (!map->active)
        return 0;
    if (munmap(map->base, map->mapped) < 0)
        return errno;
    map->base = NULL;
    map->mapped = map->logical = 0;
    map->active = 0;
    return 0;
}

static void put(unsigned char *p, uint64_t value, unsigned count,
                unsigned endian)
{
    for (unsigned i = 0; i < count; ++i)
        p[endian == 1 ? i : count - 1 - i] = (unsigned char)(value >> (8 * i));
}

static uint64_t get(const unsigned char *p, unsigned count, unsigned endian)
{
    uint64_t value = 0;
    for (unsigned i = 0; i < count; ++i)
        value |= (uint64_t)p[endian == 1 ? i : count - 1 - i] << (8 * i);
    return value;
}

static uint32_t sum(const unsigned char *p, size_t n)
{
    uint32_t v = 0;
    for (size_t i = 0; i < n; ++i)
        v = v * 33u + p[i];
    return v;
}

int pm_encode(unsigned char bytes[128], const struct pm_record *record,
              unsigned endian)
{
    if (!bytes || !record || !record->session || record->length > 64 ||
        (endian != 1 && endian != 2))
        return EINVAL;
    unsigned char temp[128] = {0};
    memcpy(temp, "PMR1", 4);
    temp[4] = 1;
    temp[5] = 1;
    temp[6] = (unsigned char)endian;
    put(temp + 8, record->session, 8, endian);
    put(temp + 16, 32, 4, endian);
    put(temp + 20, record->length, 4, endian);
    memcpy(temp + 32, record->payload, record->length);
    put(temp + 24, sum(temp + 32, record->length), 4, endian);
    memcpy(bytes, temp, 128);
    return 0;
}

int pm_decode(const unsigned char *bytes, size_t extent, struct pm_record *out)
{
    if (!bytes || !out)
        return EINVAL;
    if (extent != 128)
        return EPROTO;
    if (bytes[4] == 0 && PM_MUTANT != 3)
        return EAGAIN;
    if (bytes[4] > 1)
        return EPROTO;
    unsigned endian = PM_MUTANT == 4 ? 1 : bytes[6];
    if (memcmp(bytes, "PMR1", 4) || bytes[5] != 1 ||
        (endian != 1 && endian != 2) || bytes[7] || bytes[28] || bytes[29] ||
        bytes[30] || bytes[31])
        return EPROTO;
    uint64_t session = get(bytes + 8, 8, endian),
             offset = get(bytes + 16, 4, endian),
             length = get(bytes + 20, 4, endian);
    if (!session || offset < 32 || offset > extent ||
        length > extent - offset || length > 64)
        return EPROTO;
    if (get(bytes + 24, 4, endian) !=
        sum(bytes + (size_t)offset, (size_t)length))
        return EPROTO;
    struct pm_record result = {session, (size_t)length, {0}};
    memcpy(result.payload, bytes + (size_t)offset, (size_t)length);
    *out = result;
    return 0;
}

static int lock_record(int fd, short type, int blocking)
{
    struct flock lock = {0};
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    for (;;)
    {
        if (fcntl(fd, blocking ? F_SETLKW : F_SETLK, &lock) == 0)
            return 0;
        if (errno == EINTR)
            continue;
        return errno;
    }
}

static int map_record(int fd, unsigned char **bytes)
{
    struct stat st;
    if (fstat(fd, &st) < 0)
        return errno;
    if (st.st_size != 128)
        return EPROTO;
    void *p = mmap(NULL, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
        return errno;
    *bytes = p;
    return 0;
}

static int publish_locked(int fd, uint64_t session, const void *data,
                          size_t length, pm_cut_fn cut, void *context,
                          int replace, uint64_t expected)
{
    struct stat st;
    if (fstat(fd, &st) < 0)
        return errno;
    if (st.st_size == 0)
    {
        if (ftruncate(fd, 128) < 0)
            return errno;
    }
    else if (st.st_size != 128)
        return EPROTO;
    unsigned char *bytes = NULL;
    int e = map_record(fd, &bytes);
    if (e)
        return e;
    struct pm_record old;
    int decoded = pm_decode(bytes, 128, &old);
    if (!decoded)
    {
        if (!replace)
        {
            munmap(bytes, 128);
            return EEXIST;
        }
        if (old.session != expected)
        {
            munmap(bytes, 128);
            return ESTALE;
        }
    }
    else if (replace)
    {
        munmap(bytes, 128);
        return decoded;
    }
    else if (decoded != EAGAIN)
    {
        munmap(bytes, 128);
        return decoded;
    }
    struct pm_record record = {session, length, {0}};
    if (length)
        memcpy(record.payload, data, length);
    unsigned char temp[128];
    e = pm_encode(temp, &record, 1);
    if (e)
    {
        munmap(bytes, 128);
        return e;
    }
    bytes[4] = 0;
    if (cut)
        cut(1, context);
    memcpy(bytes, temp, 4);
    memcpy(bytes + 5, temp + 5, 123);
    if (cut)
        cut(2, context);
    bytes[4] = 1;
    if (cut)
        cut(3, context);
    return munmap(bytes, 128) == 0 ? 0 : errno;
}

int pm_publish(int fd, uint64_t session, const void *data, size_t length,
               pm_cut_fn cut, void *context)
{
    if (!session || length > 64 || (!data && length))
        return EINVAL;
    int e = lock_record(fd, F_WRLCK, 1);
    if (e)
        return e;
    e = publish_locked(fd, session, data, length, cut, context, 0, 0);
    int unlock = lock_record(fd, F_UNLCK, 0);
    return e ? e : unlock;
}

int pm_read_record(int fd, uint64_t expected, struct pm_record *out)
{
    if (!expected || !out)
        return EINVAL;
    int e = PM_MUTANT == 5 ? 0 : lock_record(fd, F_RDLCK, 0);
    if (e)
        return e;
    unsigned char *bytes = NULL;
    struct pm_record result = {0};
    e = map_record(fd, &bytes);
    if (!e)
    {
        e = pm_decode(bytes, 128, &result);
        int unmapped = munmap(bytes, 128);
        if (!e && unmapped < 0)
            e = errno;
    }
    int unlock = PM_MUTANT == 5 ? 0 : lock_record(fd, F_UNLCK, 0);
    if (!e)
        e = unlock;
    if (!e && result.session != expected && PM_MUTANT != 6)
        e = ESTALE;
    if (!e)
        *out = result;
    return e;
}

int pm_replace(int fd, uint64_t expected, uint64_t replacement,
               const void *data, size_t length)
{
    if (!expected || replacement <= expected || length > 64 ||
        (!data && length))
        return EINVAL;
    int e = lock_record(fd, F_WRLCK, 1);
    if (e)
        return e;
    e = publish_locked(fd, replacement, data, length, NULL, NULL, 1, expected);
    int unlock = lock_record(fd, F_UNLCK, 0);
    return e ? e : unlock;
}
