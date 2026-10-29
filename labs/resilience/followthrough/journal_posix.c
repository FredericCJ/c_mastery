#include "journal_posix.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static int file_size(void *p, size_t *out)
{
    struct stat info;
    if (fstat(((struct rzj_posix *)p)->descriptor, &info) || info.st_size < 0 ||
        (uintmax_t)info.st_size > SIZE_MAX)
        return RZJ_UNCERTAIN;
    *out = (size_t)info.st_size;
    return RZ_OK;
}

static int file_read(void *p, size_t offset, unsigned char *bytes, size_t n,
                     size_t *out)
{
    struct rzj_posix *f = p;
    if (offset > RZJ_MAX_RECORDS * RZJ_RECORD_BYTES ||
        n > RZJ_MAX_RECORDS * RZJ_RECORD_BYTES - offset)
        return RZ_INVALID;
    if (f->max_io && n > f->max_io)
        n = f->max_io;
    ssize_t count;
    do
    {
        count = pread(f->descriptor, bytes, n, (off_t)offset);
    } while (count < 0 && errno == EINTR);
    if (count < 0)
        return RZJ_UNCERTAIN;
    *out = (size_t)count;
    return RZ_OK;
}

static int file_write(void *p, size_t offset, const unsigned char *bytes,
                      size_t n, size_t *out)
{
    struct rzj_posix *f = p;
    if (offset > RZJ_MAX_RECORDS * RZJ_RECORD_BYTES ||
        n > RZJ_MAX_RECORDS * RZJ_RECORD_BYTES - offset)
        return RZ_INVALID;
    if (f->max_io && n > f->max_io)
        n = f->max_io;
    ssize_t count;
    do
    {
        count = pwrite(f->descriptor, bytes, n, (off_t)offset);
    } while (count < 0 && errno == EINTR);
    if (count < 0)
        return RZJ_UNCERTAIN;
    *out = (size_t)count;
    return RZ_OK;
}

static int file_truncate(void *p, size_t n)
{
    if (n > RZJ_MAX_RECORDS * RZJ_RECORD_BYTES)
        return RZ_INVALID;
    return ftruncate(((struct rzj_posix *)p)->descriptor, (off_t)n)
               ? RZJ_UNCERTAIN
               : RZ_OK;
}

static int file_flush(void *p)
{
    return fdatasync(((struct rzj_posix *)p)->descriptor) ? RZJ_UNCERTAIN
                                                          : RZ_OK;
}

int rzj_posix(int fd, size_t maximum, struct rzj_posix *out, struct rzj_io *io)
{
    if (fd < 0 || !out || !io || maximum > RZJ_MAX_RECORDS * RZJ_RECORD_BYTES)
        return RZ_INVALID;
    int flags = fcntl(fd, F_GETFL);
    struct stat info;
    if (flags < 0 || (flags & O_ACCMODE) != O_RDWR || (flags & O_APPEND) ||
        fstat(fd, &info) || !S_ISREG(info.st_mode))
        return RZ_INVALID;
    *out = (struct rzj_posix){fd, maximum};
    *io = (struct rzj_io){out,        file_size,     file_read,
                          file_write, file_truncate, file_flush};
    return RZ_OK;
}
