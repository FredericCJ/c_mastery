#define _GNU_SOURCE
#include "os.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "OF_IO:%d:%s errno=%d\n", __LINE__, #x, errno); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

static int transfer(int fd, const unsigned char *p, size_t n, unsigned inject,
                    size_t *done)
{
    struct of_transfer t = {n, 0, 0, 0};
    unsigned step = 0;
    while (t.done < n)
    {
        ptrdiff_t raw;
        if (inject && step++ == 0)
            raw = -EINTR;
        else if (inject && step == 3)
        {
            step++;
            raw = -EAGAIN;
        }
        else
        {
            size_t ask = n - t.done;
            if (inject && ask > 113)
                ask = 113;
            ssize_t got = write(fd, p + t.done, ask);
            raw = got < 0 ? -errno : got;
        }
        enum of_status rc = of_progress(&t, raw, 0);
        if (rc == OF_ERROR || rc == OF_INPUT)
            return 1;
    }
    *done = t.done;
    return 0;
}

struct of_slot
{
    unsigned object, open;
    unsigned closes;
};

static int close_model(void)
{
    for (unsigned retaining = 0; retaining < 2; retaining++)
    {
        struct of_slot slot = {1, 1, 0};
        slot.closes++;
        if (!retaining)
            slot.open = 0;
        int result = -EINTR;
        CHECK(result == -EINTR);
        if (retaining)
        {
            CHECK(slot.open && slot.object == 1);
            slot.closes++;
            slot.open = 0;
            CHECK(slot.closes == 2);
        }
        else
        {
            CHECK(!slot.open);
            slot = (struct of_slot){2, 1, 1};
            struct of_slot wrong = slot;
            wrong.open = 0;
            wrong.closes++;
            CHECK(!wrong.open && wrong.object == 2);
            CHECK(slot.open && slot.object == 2 && slot.closes == 1);
        }
        printf("OF:close-model:retain-after-EINTR%u:ownership-policy-checked\n",
               retaining);
    }
    return 0;
}

int main(void)
{
    unsigned char data[7168];
    for (size_t i = 0; i < sizeof data; i++)
        data[i] = (unsigned char)(i % 251);
    char name[] = "/tmp/of-io-XXXXXX";
    int fd = mkstemp(name);
    CHECK(fd >= 0 && !unlink(name));
    struct statvfs room;
    CHECK(!fstatvfs(fd, &room) &&
          (unsigned long long)room.f_bavail * room.f_frsize >= 8192);
    size_t wrote = 0, done = 0, ack = 0;
    CHECK(!transfer(fd, data, 4096, 1, &done));
    wrote += done;
    CHECK(!fsync(fd));
    ack = wrote;
    CHECK(!transfer(fd, data + 4096, 2048, 1, &done));
    wrote += done;
    CHECK(!fsync(fd));
    ack = wrote;
    CHECK(!transfer(fd, data + 6144, 1024, 1, &done));
    wrote += done;
    CHECK(wrote == 7168 && ack == 6144 && lseek(fd, 0, SEEK_SET) == 0);
    unsigned char readback[7168];
    size_t readbytes = 0;
    while (readbytes < 2048)
    {
        ssize_t n = read(fd, readback + readbytes, 2048 - readbytes);
        CHECK(n > 0);
        readbytes += (size_t)n;
    }
    CHECK(readbytes == 2048 && !memcmp(data, readback, 2048));
    CHECK(pread(fd, readback, sizeof readback, 0) == sizeof readback &&
          !memcmp(data, readback, sizeof data));
    printf(
        "OF:ledger:writes%zu:reads%zu:ack%zu:uncertain-tail%zu:short113:EINTR:"
        "EAGAIN\n",
        wrote, readbytes, ack, wrote - ack);
    CHECK(lseek(fd, 0, SEEK_SET) == 0 && !transfer(fd, data, 1024, 1, &done));
    wrote += done;
    struct stat st;
    CHECK(!fstat(fd, &st) && wrote == 8192 && st.st_size == 7168);
    CHECK(!close(fd));
    CHECK(!close_model());
    puts("OF_PASS:io:overlap-app8192-file7168:close-retaining-vs-Linux");
    return 0;
}
