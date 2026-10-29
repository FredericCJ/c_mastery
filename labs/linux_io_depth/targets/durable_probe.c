#define _GNU_SOURCE
#include "durable.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define C(x)                                                                 \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "B06_DURABLE_ASSERT:%d:%s errno=%d\n", __LINE__, \
                    #x, errno);                                              \
            exit(1);                                                         \
        }                                                                    \
    } while (0)
static unsigned syncs;
static ino_t synced[3];
int __real_fsync(int);

int __wrap_fsync(int fd)
{
    struct stat s;
    C(fstat(fd, &s) == 0);
    if (syncs < 3)
    {
        synced[syncs] = s.st_ino;
    }
    syncs++;
    return __real_fsync(fd);
}

static unsigned char data[4096];

static void fill(int newer)
{
    for (unsigned i = 0; i < 4096; i++)
    {
        data[i] = (unsigned char)((i + (newer ? 17 : 0)) % 251);
    }
}

static void stop(unsigned phase, void *arg)
{
    unsigned cut = *(unsigned *)arg;
    if (phase == cut)
    {
        if (cut == 5)
        {
            C(syncs == 3 && synced[1] != synced[2]);
        }
        printf("B06_DURABLE_CUT phase=%u syncs=%u\n", phase, syncs);
        fflush(stdout);
        for (;;)
        {
            pause();
        }
    }
}

static int check(int d)
{
    int f = openat(d, "target", O_RDONLY);
    C(f >= 0);
    unsigned char bytes[4096], extra;
    size_t done = 0;
    while (done < 4096)
    {
        ssize_t n = read(f, bytes + done, 4096 - done);
        C(n > 0);
        done += (size_t)n;
    }
    C(read(f, &extra, 1) == 0 && close(f) == 0);
    fill(0);
    if (!memcmp(bytes, data, 4096))
    {
        return 0;
    }
    fill(1);
    C(!memcmp(bytes, data, 4096));
    return 1;
}

int main(int argc, char **argv)
{
    C(argc >= 3);
    C(chdir(argv[2]) == 0);
    if (!strcmp(argv[1], "setup"))
    {
        C(mkdir("a", 0700) == 0 && mkdir("b", 0700) == 0);
        int a = open("a", O_RDONLY | O_DIRECTORY),
            b = open("b", O_RDONLY | O_DIRECTORY),
            root = open(".", O_RDONLY | O_DIRECTORY);
        C(a >= 0 && b >= 0 && root >= 0);
        int f = openat(b, "target", O_CREAT | O_EXCL | O_WRONLY, 0600);
        C(f >= 0);
        fill(0);
        C(write(f, data, 4096) == 4096 && fsync(f) == 0 && close(f) == 0);
        C(fsync(a) == 0 && fsync(b) == 0 && fsync(root) == 0);
        close(a);
        close(b);
        close(root);
        puts("B06_DURABLE_SETUP");
        return 0;
    }
    int a = open("a", O_RDONLY | O_DIRECTORY),
        b = open("b", O_RDONLY | O_DIRECTORY);
    C(a >= 0 && b >= 0);
    if (!strcmp(argv[1], "recover"))
    {
        C(argc == 4);
        int value = check(b);
        if (atoi(argv[3]) == 5)
        {
            C(value == 1);
        }
        printf("B06_DURABLE_RECOVER value=%s cut=%s\n", value ? "new" : "old",
               argv[3]);
    }
    else if (!strcmp(argv[1], "write"))
    {
        C(argc == 4);
        unsigned cut = (unsigned)atoi(argv[3]), phase;
        fill(1);
        syncs = 0;
        C(du_replace(a, b, data, 4096, 0, &phase, stop, &cut) == 0);
        C(phase == 5 && syncs == 3 && synced[1] != synced[2]);
        puts("B06_DURABLE_COMMIT");
    }
    else if (!strcmp(argv[1], "fault"))
    {
        C(argc == 4);
        unsigned fault = (unsigned)atoi(argv[3]), phase;
        fill(1);
        syncs = 0;
        C(du_replace(a, b, data, 4096, fault, &phase, NULL, NULL) == -EIO &&
          phase == fault - 1);
        C(check(b) == (fault > 3));
        printf("B06_DURABLE_FAILURE before=%u phase=%u visible=%s\n", fault,
               phase, fault > 3 ? "new" : "old");
    }
    else
    {
        C(0);
    }
    close(a);
    close(b);
    return 0;
}
