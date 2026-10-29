#define _POSIX_C_SOURCE 200809L
#include "store_file.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static int transfer(store_file *f, uint32_t page, void *buf, int writing)
{
    size_t done = 0;
    off_t off;
    if (page >= f->pages)
    {
        return -1;
    }
    off = (off_t)page * (off_t)STORE_PAGE_SIZE;
    while (done < STORE_PAGE_SIZE)
    {
        ssize_t n = writing ? pwrite(f->fd, (unsigned char *)buf + done,
                                     STORE_PAGE_SIZE - done, off + (off_t)done)
                            : pread(f->fd, (unsigned char *)buf + done,
                                    STORE_PAGE_SIZE - done, off + (off_t)done);
        if (n < 0 && errno == EINTR)
        {
            continue;
        }
        if (n <= 0)
        {
            return -1;
        }
        done += (size_t)n;
    }
    return 0;
}

static int file_read(void *ctx, uint32_t p, void *b)
{
    return transfer(ctx, p, b, 0);
}

static int file_write(void *ctx, uint32_t p, const void *b)
{
    return transfer(ctx, p, (void *)b, 1);
}

static int file_flush(void *ctx)
{
    store_file *f = ctx;
    int r;
    do
    {
        r = fsync(f->fd);
    } while (r < 0 && errno == EINTR);
    return r;
}

int store_file_open(store_file *f, const char *path, uint32_t pages)
{
    struct stat st;
    if (!f || !path || pages < STORE_IMAGE_PAGES)
    {
        return STORE_INVALID;
    }
    f->fd = open(path, O_RDWR);
    f->pages = pages;
    if (f->fd < 0)
    {
        return STORE_IO;
    }
    if (fstat(f->fd, &st) || !S_ISREG(st.st_mode) ||
        st.st_size < (off_t)pages * (off_t)STORE_PAGE_SIZE)
    {
        close(f->fd);
        f->fd = -1;
        return STORE_INVALID;
    }
    return STORE_OK;
}

void store_file_close(store_file *f)
{
    if (f && f->fd >= 0)
    {
        close(f->fd);
        f->fd = -1;
    }
}

store_io store_file_io(store_file *f)
{
    store_io io = {f, file_read, file_write, file_flush, f->pages};
    return io;
}
