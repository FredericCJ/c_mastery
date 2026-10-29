#define _GNU_SOURCE
#include "transfer.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define C(x)                                                                \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "B06_DIRECT_ASSERT:%d:%s errno=%d\n", __LINE__, \
                    #x, errno);                                             \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

static uint64_t ns(void)
{
    struct timespec t;
    C(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (uint64_t)t.tv_sec * 1000000000 + (uint64_t)t.tv_nsec;
}

static void all(int fd, const unsigned char *p, size_t n)
{
    size_t done = 0;
    while (done < n)
    {
        ssize_t k = pwrite(fd, p + done, n - done, (off_t)done);
        if (k < 0 && errno == EINTR)
        {
            continue;
        }
        C(k > 0);
        done += (size_t)k;
    }
    C(fdatasync(fd) == 0);
}

int main(void)
{
    char dir[PATH_MAX];
    const char *base = getenv("B06_DIRECT_PARENT");
    if (!base)
    {
        base = "/tmp";
    }
    int extent = snprintf(dir, sizeof dir, "%s/cm-b06-direct-XXXXXX", base);
    C(extent > 0 && (size_t)extent < sizeof dir);
    C(mkdtemp(dir));
    int d = open(dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    C(d >= 0);
    int f = openat(d, "probe", O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
    C(f >= 0);
    struct statx sx;
    C(statx(f, "", AT_EMPTY_PATH, STATX_DIOALIGN, &sx) == 0);
    C((sx.stx_mask & STATX_DIOALIGN) && sx.stx_dio_mem_align &&
      sx.stx_dio_offset_align);
    unsigned ma = sx.stx_dio_mem_align, oa = sx.stx_dio_offset_align;
    printf("direct_alignment memory=%u offset_length=%u mask=%u\n", ma, oa,
           sx.stx_mask);
    C(ma <= 65536 && oa <= 65536 && (ma & (ma - 1)) == 0);
    C(close(f) == 0 && unlinkat(d, "probe", 0) == 0);
    size_t n = oa > 4096 ? oa : 4096;
    while (n % oa)
    {
        n++;
    }
    void *aligned = NULL;
    C(posix_memalign(&aligned, ma < sizeof(void *) ? sizeof(void *) : ma, n) ==
      0);
    unsigned char *misbase = malloc(n + 1);
    C(misbase);
    unsigned char *mis = misbase + 1;
    for (size_t i = 0; i < n; i++)
    {
        ((unsigned char *)aligned)[i] = mis[i] = (unsigned char)(i % 251);
    }
    C(it_direct_ok((uintptr_t)aligned, n, 0, ma, oa));
    C(!it_direct_ok((uintptr_t)mis, n, 0, ma, oa));
    for (unsigned trial = 0; trial < 8; trial++)
    {
        for (unsigned mode = 0; mode < 2; mode++)
        {
            const unsigned char *p = mode ? mis : aligned;
            int chosen = it_direct_ok((uintptr_t)p, n, 0, ma, oa);
            C(chosen == (mode == 0));
            char name[32];
            C(snprintf(name, sizeof name, "sample-%u-%u", trial, mode) > 0);
            f = openat(d, name,
                       O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC |
                           (chosen ? O_DIRECT : 0),
                       0600);
            C(f >= 0);
            uint64_t before = ns();
            all(f, p, n);
            C(fsync(d) == 0);
            uint64_t elapsed = ns() - before;
            C(close(f) == 0);
            f = openat(d, name, O_RDONLY | O_CLOEXEC);
            C(f >= 0);
            unsigned char *check = malloc(n);
            C(check);
            size_t done = 0;
            while (done < n)
            {
                ssize_t k = read(f, check + done, n - done);
                C(k > 0);
                done += (size_t)k;
            }
            C(!memcmp(check, aligned, n));
            unsigned char extra;
            C(read(f, &extra, 1) == 0);
            free(check);
            C(close(f) == 0 && unlinkat(d, name, 0) == 0);
            printf(
                "sample trial=%u path=%s bytes=%zu transfer_fdatasync_directory_fsync_ns=%llu\n",
                trial, chosen ? "direct" : "buffered-fallback", n,
                (unsigned long long)elapsed);
        }
    }
    free(aligned);
    free(misbase);
    C(fsync(d) == 0 && close(d) == 0 && rmdir(dir) == 0);
    puts("B06_DIRECT_PASS equivalent_content_and_sync_contract samples=16");
    return 0;
}
