#define _POSIX_C_SOURCE 200809L
#include "file.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/utsname.h>
#include <unistd.h>
#define CHECK(x)                                                     \
    do                                                               \
    {                                                                \
        if (!(x))                                                    \
        {                                                            \
            fprintf(stderr, "SFI_CHECK line%d: %s\n", __LINE__, #x); \
            return 1;                                                \
        }                                                            \
    } while (0)
_Static_assert(sizeof(off_t) >= 8, "selected64bit-offset profile");
ssize_t __real_pread(int, void *, size_t, off_t);
ssize_t __real_pwrite(int, const void *, size_t, off_t);
int __real_fsync(int);
int __real_close(int);

static struct
{
    int active;
    unsigned first, mode, done, calls, interrupted, bad;
    uint64_t base;
    unsigned char bytes[4096];
} hook;

static uint64_t real_read_calls, real_write_calls, accepted_read,
    accepted_write;
static unsigned sync_calls, sync_fail, sync_after, sync_intr, close_calls,
    close_fail, dir_syncs, file_syncs;
static unsigned sync_kinds[256];

static ptrdiff_t transfer(void *out, const void *in, size_t n, uint64_t offset)
{
    hook.calls++;
    if (offset != hook.base + hook.done || n != 4096 - hook.done)
    {
        hook.bad = 1;
        errno = EIO;
        return -1;
    }
    if (!hook.interrupted && ((hook.mode == 1 && hook.done == 0) ||
                              (hook.mode == 2 && hook.done > 0)))
    {
        hook.interrupted = 1;
        errno = EINTR;
        return -1;
    }
    if (hook.mode >= 3 && hook.done == hook.first)
    {
        errno = EIO;
        return hook.mode == 3 ? 0 : -1;
    }
    size_t take = hook.done ? n : hook.first;
    if (take > n)
    {
        hook.bad = 1;
        errno = EIO;
        return -1;
    }
    if (out)
        memcpy(out, hook.bytes + hook.done, take);
    else if (memcmp(in, hook.bytes + hook.done, take))
    {
        hook.bad = 1;
        errno = EIO;
        return -1;
    }
    hook.done += (unsigned)take;
    return (ptrdiff_t)take;
}

ssize_t __wrap_pread(int fd, void *p, size_t n, off_t off)
{
    if (hook.active && fd == 12345)
        return (ssize_t)transfer(p, NULL, n, (uint64_t)off);
    ssize_t r = __real_pread(fd, p, n, off);
    real_read_calls++;
    if (r > 0)
        accepted_read += (uint64_t)r;
    return r;
}

ssize_t __wrap_pwrite(int fd, const void *p, size_t n, off_t off)
{
    if (hook.active && fd == 12345)
        return (ssize_t)transfer(NULL, p, n, (uint64_t)off);
    ssize_t r = __real_pwrite(fd, p, n, off);
    real_write_calls++;
    if (r > 0)
        accepted_write += (uint64_t)r;
    return r;
}

int __wrap_fsync(int fd)
{
    struct stat st;
    int directory = !fstat(fd, &st) && S_ISDIR(st.st_mode);
    if (sync_calls < 256)
        sync_kinds[sync_calls] = (unsigned)directory;
    sync_calls++;
    if (directory)
        dir_syncs++;
    else
        file_syncs++;
    if (sync_intr)
    {
        sync_intr--;
        errno = EINTR;
        return -1;
    }
    int result = 0;
    if (sync_calls != sync_fail || sync_after)
        result = __real_fsync(fd);
    if (sync_calls == sync_fail)
    {
        errno = EIO;
        return -1;
    }
    return result;
}

int __wrap_close(int fd)
{
    close_calls++;
    int r = __real_close(fd);
    if (close_fail)
    {
        close_fail = 0;
        errno = EINTR;
        return -1;
    }
    return r;
}

static void counts(void)
{
    real_read_calls = real_write_calls = accepted_read = accepted_write = 0;
    sync_calls = sync_fail = sync_after = sync_intr = close_calls = close_fail =
        dir_syncs = file_syncs = 0;
    memset(sync_kinds, 0, sizeof sync_kinds);
}

static void pattern(unsigned char *p)
{
    for (unsigned i = 0; i < 4096; i++)
        p[i] = (unsigned char)(i * 17u + i / 251u);
}

static int original_transfer(void)
{
    unsigned cases = 0;
    unsigned char out[4096];
    store_file f = {12345, UINT32_MAX};
    store_io io = store_file_io(&f);
    pattern(hook.bytes);
    for (unsigned writing = 0; writing < 2; writing++)
        for (unsigned mode = 0; mode < 5; mode++)
            for (unsigned first = mode < 3 ? 1 : 0; first < 4096; first++)
            {
                hook.active = 1;
                hook.first = first;
                hook.mode = mode;
                hook.done = hook.calls = hook.interrupted = hook.bad = 0;
                hook.base = (uint64_t)(UINT32_MAX - 1u) * 4096u;
                memset(out, 0xa5, 4096);
                int r = writing ? io.write(io.ctx, UINT32_MAX - 1u, hook.bytes)
                                : io.read(io.ctx, UINT32_MAX - 1u, out);
                CHECK(!hook.bad && hook.calls <= 3 &&
                      (mode < 3 ? r == 0 && hook.done == 4096
                                : r != 0 && hook.done == first));
                if (!writing)
                    for (unsigned i = 0; i < 4096; i++)
                        CHECK(out[i] == (i < hook.done ? hook.bytes[i] : 0xa5));
                cases++;
            }
    hook.active = 0;
    printf("SFI original_transfer validated histories=%u all-positive-shorts "
           "EINTR-before-and-after zero-error-prefixes wide-offset\n",
           cases);
    return 0;
}

struct store_bytes
{
    unsigned char bytes[4096];
    size_t done, chunk;
    unsigned calls, retry, again, zero, oversize;
    uint64_t base;
};

static ptrdiff_t read_bytes(void *ctx, uint64_t off, void *p, size_t n)
{
    struct store_bytes *b = ctx;
    b->calls++;
    if (off != b->base + b->done || n != 4096 - b->done)
        return -1;
    if (b->retry)
    {
        b->retry--;
        return SFI_INTR;
    }
    if (b->again)
    {
        b->again--;
        return SFI_AGAIN;
    }
    if (b->zero)
        return 0;
    if (b->oversize)
        return (ptrdiff_t)n + 1;
    if (n > b->chunk)
        n = b->chunk;
    memcpy(p, b->bytes + b->done, n);
    b->done += n;
    return (ptrdiff_t)n;
}

static ptrdiff_t write_bytes(void *ctx, uint64_t off, const void *p, size_t n)
{
    struct store_bytes *b = ctx;
    b->calls++;
    if (off != b->base + b->done || n != 4096 - b->done)
        return -1;
    if (b->retry)
    {
        b->retry--;
        return SFI_INTR;
    }
    if (b->again)
    {
        b->again--;
        return SFI_AGAIN;
    }
    if (b->zero)
        return 0;
    if (b->oversize)
        return (ptrdiff_t)n + 1;
    if (n > b->chunk)
        n = b->chunk;
    memcpy(b->bytes + b->done, p, n);
    b->done += n;
    return (ptrdiff_t)n;
}

static int nonblocking(int oracle)
{
    unsigned char input[4096], output[4096];
    pattern(input);
    unsigned cases = 0;
    for (unsigned writing = 0; writing < 2; writing++)
        for (unsigned chunk = 1; chunk <= 4096; chunk = chunk == 1      ? 127
                                                        : chunk == 127  ? 4095
                                                        : chunk == 4095 ? 4096
                                                                        : 4097)
        {
            struct sfi_op op;
            sfi_init(&op);
            struct sfi_ticket t;
            struct store_bytes b = {0};
            b.chunk = chunk;
            b.base = UINT64_C(7) * 4096;
            b.retry = 2;
            b.again = 2;
            if (!writing)
                memcpy(b.bytes, input, 4096);
            memset(output, 0xa5, 4096);
            CHECK(sfi_begin(&op,
                            (struct sfi_io){&b, read_bytes, write_bytes, 8},
                            writing, 7, input, output, &t) == SFI_OK);
            CHECK(sfi_step(t, 0, 0) == SFI_BUDGET && b.calls == 0 &&
                  sfi_release(t) == SFI_BUSY);
            CHECK(sfi_step(t, 0, 1) == SFI_BUDGET &&
                  sfi_step(t, 0, 1) == SFI_BUDGET && b.calls == 2);
            CHECK(sfi_step(t, 0, 9) == SFI_WAIT && b.calls == 3);
            CHECK(sfi_step(t, 0, 9) == SFI_WAIT && b.calls == 3);
            CHECK(sfi_step(t, 1, 9) == SFI_WAIT && b.calls == 4);
            CHECK(sfi_step(t, 0, 9) == SFI_WAIT && b.calls == 4);
            int r;
            do
            {
                r = sfi_step(t, 1, 3);
                CHECK(r == SFI_BUDGET || r == SFI_DONE);
            } while (r == SFI_BUDGET);
            CHECK(op.done == 4096 && b.done == 4096 &&
                  memcmp(writing ? b.bytes : output, input, 4096) == 0);
            unsigned calls = b.calls;
            CHECK(sfi_step(t, 0, 9) == SFI_DONE && b.calls == calls &&
                  sfi_release(t) == SFI_OK && sfi_step(t, 1, 9) == SFI_STALE);
            if (oracle)
                printf("%u %u %u %zu\n", writing, chunk, calls, op.done);
            cases++;
        }
    for (unsigned mode = 0; mode < 3; mode++)
    {
        struct sfi_op op;
        sfi_init(&op);
        struct sfi_ticket t;
        struct store_bytes b = {0};
        pattern(b.bytes);
        b.chunk = 17;
        memset(output, 0xa5, 4096);
        CHECK(sfi_begin(&op, (struct sfi_io){&b, read_bytes, write_bytes, 1}, 0,
                        0, NULL, output, &t) == SFI_OK &&
              sfi_step(t, 0, 1) == SFI_BUDGET && op.done == 17);
        if (mode == 0)
            CHECK(sfi_cancel(t) == SFI_CANCELLED &&
                  sfi_step(t, 1, 9) == SFI_CANCELLED);
        else
        {
            b.zero = mode == 1;
            b.oversize = mode == 2;
            CHECK(sfi_step(t, 0, 1) == SFI_IO && op.done == 17);
        }
        for (unsigned i = 0; i < 4096; i++)
            CHECK(output[i] == 0xa5);
        CHECK(sfi_release(t) == SFI_OK);
    }
    struct sfi_op op;
    sfi_init(&op);
    op.generation = UINT64_MAX - 1;
    struct sfi_ticket t, old;
    struct store_bytes b = {0};
    b.chunk = 4096;
    CHECK(sfi_begin(&op, (struct sfi_io){&b, read_bytes, write_bytes, 1}, 1, 0,
                    input, NULL, &t) == SFI_OK &&
          t.generation == UINT64_MAX);
    old = t;
    CHECK(sfi_cancel(t) == SFI_CANCELLED && sfi_release(t) == SFI_OK &&
          sfi_begin(&op, (struct sfi_io){&b, read_bytes, write_bytes, 1}, 1, 0,
                    input, NULL, &t) == SFI_FULL &&
          sfi_step(old, 1, 1) == SFI_STALE);
    if (!oracle)
        printf("SFI nonblocking validated cases=%u "
               "readiness-budget-terminal-prefix-retirement\n",
               cases);
    return 0;
}

struct store_pipe
{
    int fd;
    uint64_t next;
    unsigned calls;
};

static ptrdiff_t pipe_read(void *ctx, uint64_t at, void *p, size_t n)
{
    struct store_pipe *a = ctx;
    a->calls++;
    if (at != a->next)
        return -1;
    ssize_t r = read(a->fd, p, n);
    if (r < 0)
        return errno == EINTR                              ? SFI_INTR
               : (errno == EAGAIN || errno == EWOULDBLOCK) ? SFI_AGAIN
                                                           : -1;
    if (r > 0)
        a->next += (uint64_t)r;
    return r;
}

static int readiness(void)
{
    int descriptors[2];
    CHECK(pipe(descriptors) == 0);
    int flags = fcntl(descriptors[0], F_GETFL);
    CHECK(flags >= 0 &&
          fcntl(descriptors[0], F_SETFL, flags | O_NONBLOCK) == 0);
    struct store_pipe p = {descriptors[0], 0, 0};
    struct sfi_op op;
    sfi_init(&op);
    struct sfi_ticket t;
    unsigned char data[4096], out[4096];
    pattern(data);
    memset(out, 0xa5, 4096);
    CHECK(sfi_begin(&op, (struct sfi_io){&p, pipe_read, NULL, 1}, 0, 0, NULL,
                    out, &t) == SFI_OK &&
          sfi_step(t, 0, 1) == SFI_WAIT && p.calls == 1);
    struct pollfd fd = {descriptors[0], POLLIN, 0};
    CHECK(poll(&fd, 1, 0) == 0 && sfi_step(t, 0, 99) == SFI_WAIT &&
          p.calls == 1);
    CHECK(write(descriptors[1], data, 13) == 13 && poll(&fd, 1, 1000) == 1 &&
          (fd.revents & POLLIN) && sfi_step(t, 1, 1) == SFI_BUDGET &&
          op.done == 13);
    CHECK(sfi_step(t, 0, 1) == SFI_WAIT);
    CHECK(write(descriptors[1], data + 13, 4083) == 4083 &&
          close(descriptors[1]) == 0 && poll(&fd, 1, 1000) == 1 &&
          (fd.revents & POLLIN) && (fd.revents & POLLHUP) &&
          sfi_step(t, 1, 1) == SFI_DONE && memcmp(data, out, 4096) == 0 &&
          sfi_release(t) == SFI_OK && close(descriptors[0]) == 0);
    puts(
        "SFI readiness validated actual-nonblocking-pipe-poll-prefix-and-hangup");
    return 0;
}

static int create_file(char path[64])
{
    strcpy(path, "/tmp/cmastery-sfi-XXXXXX");
    int fd = mkstemp(path);
    CHECK(fd >= 0 &&
          posix_fallocate(fd, 0, (off_t)STORE_IMAGE_PAGES * 4096) == 0 &&
          close(fd) == 0);
    return 0;
}

static int get(store *s, unsigned want)
{
    unsigned char key = 1, value = 0;
    size_t n = 0;
    return store_get(s, &key, 1, &value, 1, &n) == STORE_OK && n == 1 &&
           value == want;
}

static int update(store *s, unsigned value)
{
    unsigned char key = 1, v = (unsigned char)value;
    CHECK(store_begin(s) == STORE_OK &&
          store_put(s, &key, 1, &v, 1) == STORE_OK);
    return 0;
}

static int native(void)
{
    char path[64];
    CHECK(create_file(path) == 0);
    store_file f;
    store s;
    CHECK(store_file_open(&f, path, STORE_IMAGE_PAGES) == STORE_OK);
    store_io io = store_file_io(&f);
    CHECK(store_format(io) == STORE_OK && store_open(&s, io) == STORE_OK &&
          update(&s, 11) == 0);
    counts();
    sync_intr = 1;
    CHECK(store_commit(&s) == STORE_OK && sync_calls == 4);
    store_file_close(&f);
    CHECK(f.fd == -1 &&
          store_file_open(&f, path, STORE_IMAGE_PAGES) == STORE_OK &&
          store_open(&s, store_file_io(&f)) == STORE_OK && get(&s, 11));
    CHECK(update(&s, 22) == 0);
    counts();
    sync_fail = 3;
    sync_after = 1;
    CHECK(store_commit(&s) == STORE_INDETERMINATE && s.poisoned);
    sync_fail = 0;
    store_file_close(&f);
    CHECK(store_file_open(&f, path, STORE_IMAGE_PAGES) == STORE_OK &&
          store_open(&s, store_file_io(&f)) == STORE_OK && get(&s, 22));
    struct statfs fs;
    struct utsname os;
    CHECK(fstatfs(f.fd, &fs) == 0 && uname(&os) == 0);
    printf("SFI native validated preallocated-commit-close-reopen "
           "failed-sync-after-persist kernel=%s filesystem=%lx off_t=%zu\n",
           os.release, (unsigned long)fs.f_type, sizeof(off_t));
    store_file_close(&f);
    CHECK(unlink(path) == 0);
    return 0;
}

static int fresh(void)
{
    char path[] = "/tmp/cmastery-sfi-dir-XXXXXX";
    CHECK(mkdtemp(path) != NULL);
    int directory = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    CHECK(directory >= 0);
    store_file f = {-77, 99}, old = f;
    counts();
    CHECK(sfi_create(directory, "image", STORE_IMAGE_PAGES, &f) == SFI_OK &&
          f.fd >= 0 && file_syncs == 4 && dir_syncs == 1 && sync_kinds[4] == 1);
    store s;
    CHECK(store_open(&s, store_file_io(&f)) == STORE_OK &&
          update(&s, 19) == 0 && store_commit(&s) == STORE_OK);
    CHECK(sfi_close(&f) == SFI_OK && f.fd == -1);
    f = old;
    CHECK(sfi_create(directory, "image", STORE_IMAGE_PAGES, &f) == SFI_EXISTS &&
          f.fd == old.fd && f.pages == old.pages);
    counts();
    sync_fail = 5;
    CHECK(sfi_create(directory, "uncertain", STORE_IMAGE_PAGES, &f) ==
              SFI_UNCERTAIN &&
          f.fd == old.fd && f.pages == old.pages && close_calls == 1);
    sync_fail = 0;
    int fd = openat(directory, "uncertain", O_RDWR);
    CHECK(fd >= 0);
    store_file recovered = {fd, STORE_IMAGE_PAGES};
    CHECK(store_open(&s, store_file_io(&recovered)) == STORE_OK);
    counts();
    close_fail = 1;
    CHECK(sfi_close(&recovered) == SFI_UNCERTAIN && recovered.fd == -1 &&
          close_calls == 1 && sfi_close(&recovered) == SFI_INVALID &&
          close_calls == 1);
    CHECK(sfi_create(directory, "../escape", STORE_IMAGE_PAGES, &f) ==
              SFI_INVALID &&
          f.fd == old.fd);
    CHECK(unlinkat(directory, "image", 0) == 0 &&
          unlinkat(directory, "uncertain", 0) == 0 && close(directory) == 0 &&
          rmdir(path) == 0);
    puts(
        "SFI fresh validated file-before-directory-sync uncertain-name-retained "
        "close-not-retried");
    return 0;
}

static int amplification(void)
{
    char path[64];
    CHECK(create_file(path) == 0);
    store_file f;
    store s;
    CHECK(store_file_open(&f, path, STORE_IMAGE_PAGES) == STORE_OK &&
          store_format(store_file_io(&f)) == STORE_OK &&
          store_open(&s, store_file_io(&f)) == STORE_OK && update(&s, 9) == 0 &&
          store_commit(&s) == STORE_OK && store_checkpoint(&s) == STORE_OK &&
          store_open(&s, store_file_io(&f)) == STORE_OK);
    counts();
    uint64_t rd = s.reads, wr = s.writes, fl = s.flushes;
    CHECK(update(&s, 10) == 0 && store_commit(&s) == STORE_OK);
    CHECK(s.reads - rd == 0 && s.writes - wr == 34 && s.flushes - fl == 3 &&
          accepted_write == 34 * 4096 && accepted_read == 0);
    printf("{\"commit\":[%" PRIu64 ",%" PRIu64 ",%" PRIu64
           "],\"accepted_commit_bytes\":%" PRIu64 ",",
           s.reads - rd, s.writes - wr, s.flushes - fl, accepted_write);
    counts();
    rd = s.reads;
    wr = s.writes;
    fl = s.flushes;
    CHECK(store_checkpoint(&s) == STORE_OK);
    CHECK(s.reads - rd == 65 && s.writes - wr == 34 && s.flushes - fl == 3 &&
          accepted_read == 65 * 4096 && accepted_write == 34 * 4096);
    printf("\"checkpoint\":[%" PRIu64 ",%" PRIu64 ",%" PRIu64
           "],\"accepted_checkpoint_read\":%" PRIu64
           ",\"accepted_checkpoint_write\":%" PRIu64
           ",\"useful_changed_bytes\":1}\n",
           s.reads - rd, s.writes - wr, s.flushes - fl, accepted_read,
           accepted_write);
    CHECK(get(&s, 10));
    store_file_close(&f);
    CHECK(unlink(path) == 0);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "original_transfer"))
        return original_transfer();
    if (!strcmp(argv[1], "nonblocking"))
        return nonblocking(0);
    if (!strcmp(argv[1], "oracle"))
        return nonblocking(1);
    if (!strcmp(argv[1], "readiness"))
        return readiness();
    if (!strcmp(argv[1], "native"))
        return native();
    if (!strcmp(argv[1], "fresh"))
        return fresh();
    if (!strcmp(argv[1], "amplification"))
        return amplification();
    return 2;
}
