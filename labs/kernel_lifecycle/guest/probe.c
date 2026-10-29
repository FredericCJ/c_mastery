#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define CHECK(x)                                                      \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "B06_KERNEL_CHECK line=%d errno=%d %s\n", \
                    __LINE__, errno, #x);                             \
            exit(1);                                                  \
        }                                                             \
    } while (0)

static void stats(int f, uint32_t *v)
{
    CHECK(read(f, v, 24) == 24);
}

static void replacement(int f)
{
    uint32_t old[6], now[6], q[4];
    unsigned char msg[5] = {1, 1, 0, 0, 17};
    int child_status;
    pid_t p;
    CHECK(write(f, msg, 5) == 5);
    p = fork();
    CHECK(p >= 0);
    if (!p)
    {
        for (unsigned n = 0; n < 20000; n++)
        {
            uint32_t k[4] = {n % 2, (n / 2) % 2, 0, 0};
            CHECK(ioctl(f, 0xB062, k) == 0);
            CHECK(k[2] >= 1 && k[3] <= 1);
        }
        _exit(0);
    }
    for (unsigned i = 0; i < 64; i++)
    {
        int next;
        for (unsigned key = 0; key < 2; key++)
            for (unsigned mode = 0; mode < 2; mode++)
            {
                q[0] = key;
                q[1] = mode;
                q[2] = q[3] = 0;
                CHECK(ioctl(f, 0xB062, q) == 0);
                CHECK(q[2] == (key ? 1 : i + 1) && q[3] == 0);
            }
        CHECK(ioctl(f, 0xB061, 0) == 0);
        stats(f, old);
        CHECK(old[3] == 1 && old[5] == 17);
        CHECK(write(f, msg, 5) == -1 && errno == ENODEV);
        next = open("/dev/cm_b06_owner", O_RDWR);
        CHECK(next >= 0);
        stats(next, now);
        CHECK(now[0] == 0 && now[3] == 0);
        CHECK(write(next, msg, 5) == 5);
        usleep(1000);
        stats(f, now);
        CHECK(now[1] == old[1]);
        CHECK(close(f) == 0);
        f = next;
    }
    CHECK(waitpid(p, &child_status, 0) == p && WIFEXITED(child_status) &&
          WEXITSTATUS(child_status) == 0);
    CHECK(close(f) == 0);
    puts(
        "B06_REPLACEMENT_PASS generations=64 table=2 lookup_modes=2 concurrent_lookups=20000 old_fd=retained old_work=quiescent");
}

static void removal_race(int owner)
{
    int ready[2], go[2], status;
    unsigned char token;
    unsigned char msg[8] = {1, 4, 0, 0, 1, 2, 3, 4};
    CHECK(pipe(ready) == 0 && pipe(go) == 0);
    for (unsigned i = 0; i < 4; i++)
    {
        pid_t p = fork();
        CHECK(p >= 0);
        if (!p)
        {
            int h = open("/dev/cm_b06_owner", O_RDWR);
            CHECK(h >= 0);
            CHECK(write(h, msg, 8) == 8);
            CHECK(write(ready[1], "R", 1) == 1);
            CHECK(read(go[0], &token, 1) == 1);
            unsigned denied = 0;
            for (unsigned k = 0; k < 1000; k++)
            {
                ssize_t n = write(h, msg, 8);
                CHECK(n == 8 || (n == -1 && errno == ENODEV));
                if (n == -1)
                    denied++;
                int other = open("/dev/cm_b06_owner", O_RDWR);
                if (other < 0)
                {
                    CHECK(errno == ENODEV);
                    denied++;
                }
                else
                    CHECK(close(other) == 0);
            }
            CHECK(denied > 0);
            CHECK(close(h) == 0);
            _exit(0);
        }
    }
    for (unsigned i = 0; i < 4; i++)
        CHECK(read(ready[0], &token, 1) == 1);
    CHECK(write(go[1], "GGGG", 4) == 4);
    CHECK(ioctl(owner, 0xB060, 0) == 0);
    for (unsigned i = 0; i < 4; i++)
        CHECK(wait(&status) > 0 && WIFEXITED(status) &&
              WEXITSTATUS(status) == 0);
    close(ready[0]);
    close(ready[1]);
    close(go[0]);
    close(go[1]);
    CHECK(close(owner) == 0);
    puts("B06_REMOVE_RACE_PASS actors=4 attempts=4000 revoked=observed");
}

int main(int argc, char **argv)
{
    int f, g, i, status;
    uint32_t before[6], after[6];
    unsigned char msg[8] = {1, 4, 0, 0, 17, 19, 23, 29};
    f = open("/dev/cm_b06_owner", O_RDWR);
    CHECK(f >= 0);
    if (argc > 1 && !strcmp(argv[1], "replace"))
    {
        replacement(f);
        return 0;
    }
    if (argc > 1 && !strcmp(argv[1], "race"))
    {
        removal_race(f);
        return 0;
    }
    if (argc > 1 && !strcmp(argv[1], "hold"))
    {
        puts("B06_HOLD_READY");
        fflush(stdout);
        sleep(2);
        close(f);
        return 0;
    }
    g = open("/dev/cm_b06_owner", O_RDWR);
    CHECK(g >= 0);
    CHECK(write(f, msg, sizeof msg) == sizeof msg);
    stats(f, before);
    CHECK(before[0] == 1 && before[2] == 4 && before[5] == 17);
    {
        long page = sysconf(_SC_PAGESIZE);
        unsigned char *p = mmap(NULL, (size_t)page * 2, PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        CHECK(p != MAP_FAILED);
        memcpy(p + page - 4, msg, 4);
        CHECK(mprotect(p + page, (size_t)page, PROT_NONE) == 0);
        CHECK(write(f, p + page - 4, 8) == -1 && errno == EFAULT);
        stats(f, after);
        CHECK(after[0] == before[0] && after[5] == before[5]);
        CHECK(munmap(p, (size_t)page * 2) == 0);
    }
    msg[0] = 2;
    msg[2] = 1;
    CHECK(write(f, msg, 8) == 8);
    msg[3] = 1;
    CHECK(write(f, msg, 8) == -1 && errno == EINVAL);
    msg[3] = 0;
    {
        pid_t p = fork();
        CHECK(p >= 0);
        if (!p)
        {
            CHECK(setuid(1000) == 0);
            CHECK(ioctl(f, 0xB060, 0) == -1 && errno == EPERM);
            _exit(0);
        }
        CHECK(waitpid(p, &status, 0) == p && WIFEXITED(status) &&
              WEXITSTATUS(status) == 0);
    }
    for (i = 0; i < 4; ++i)
    {
        pid_t p = fork();
        CHECK(p >= 0);
        if (!p)
        {
            int k;
            for (k = 0; k < 1000; ++k)
            {
                int h = open("/dev/cm_b06_owner", O_RDWR);
                CHECK(h >= 0);
                CHECK(write(h, msg, 8) == 8);
                CHECK(close(h) == 0);
            }
            _exit(0);
        }
    }
    for (i = 0; i < 4; ++i)
        CHECK(wait(&status) > 0 && WIFEXITED(status) &&
              WEXITSTATUS(status) == 0);
    usleep(20000);
    stats(f, after);
    CHECK(after[0] == 4002 && after[1] > 0 && after[4] > 0);
    CHECK(ioctl(f, 0xB060, 0) == 0);
    CHECK(open("/dev/cm_b06_owner", O_RDWR) == -1 && errno == ENODEV);
    CHECK(write(g, msg, 8) == -1 && errno == ENODEV);
    stats(g, before);
    CHECK(before[3] == 1);
    usleep(20000);
    stats(g, after);
    CHECK(before[1] == after[1]);
    CHECK(close(f) == 0);
    CHECK(close(g) == 0);
    puts(
        "B06_KERNEL_PASS copy_fault=atomic auth=denied writes=4002 hot_unplug=revoked hardirq=observed requeue=quiescent");
    return 0;
}
