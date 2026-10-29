#define _GNU_SOURCE
#include "path.h"
#include "worker.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(x)                                   \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            fprintf(stderr, "SD_ASSERT:%s\n", #x); \
            exit(41);                              \
        }                                          \
    } while (0)

static unsigned descriptors(void)
{
    DIR *d = opendir("/proc/self/fd");
    CHECK(d);
    unsigned n = 0;
    struct dirent *e;
    while ((e = readdir(d)))
        if (strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))
            ++n;
    CHECK(!closedir(d));
    return n;
}

static void file(int d, const char *name, const char *data)
{
    int f = openat(d, name, O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC, 0600);
    CHECK(f >= 0);
    CHECK(write(f, data, 4) == 4);
    CHECK(!close(f));
}

static void content(int fd, const char *want)
{
    char b[4];
    CHECK(pread(fd, b, 4, 0) == 4 && !memcmp(b, want, 4));
}

static void wait_ok(pid_t p)
{
    int s;
    CHECK(waitpid(p, &s, 0) == p && WIFEXITED(s) && WEXITSTATUS(s) == 0);
}

static void temporary(char root[512], const char *tag)
{
    const char *base = getenv("SD_TMP_ROOT");
    if (!base)
        base = "/tmp";
    int n = snprintf(root, 512, "%s/cmastery-sd-%s-XXXXXX", base, tag);
    CHECK(n > 0 && n < 512 && mkdtemp(root));
}

static void race(void)
{
    char root[512];
    temporary(root, "race");
    int d = open(root, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    CHECK(d >= 0);
    file(d, "safe", "SAFE");
    file(d, "other", "EVIL");
    CHECK(!symlinkat("safe", d, "chosen"));
    int retained;
    CHECK(!pn_open_confined(d, "safe", &retained));
    struct stat before;
    CHECK(!fstatat(d, "chosen", &before, 0));
    int gate[2], ack[2];
    CHECK(!pipe2(gate, O_CLOEXEC) && !pipe2(ack, O_CLOEXEC));
    pid_t p = fork();
    CHECK(p >= 0);
    if (!p)
    {
        char c;
        close(gate[1]);
        close(ack[0]);
        if (read(gate[0], &c, 1) != 1 || unlinkat(d, "chosen", 0) ||
            symlinkat("other", d, "chosen") || write(ack[1], "A", 1) != 1)
            _exit(1);
        _exit(0);
    }
    CHECK(!close(gate[0]) && !close(ack[1]));
    CHECK(write(gate[1], "G", 1) == 1);
    char c;
    CHECK(read(ack[0], &c, 1) == 1);
    CHECK(!close(gate[1]) && !close(ack[0]));
    wait_ok(p);
    int flawed = openat(d, "chosen", O_RDONLY | O_CLOEXEC);
    CHECK(flawed >= 0);
    struct stat after;
    CHECK(!fstat(flawed, &after));
    CHECK(before.st_ino != after.st_ino);
    content(flawed, "EVIL");
    content(retained, "SAFE");
    CHECK(!close(flawed) && !close(retained));
    int sentinel = 77;
    CHECK(sd_exclusive(d, "chosen", &sentinel) == EEXIST && sentinel == 77);
    CHECK(!sd_exclusive(d, "fresh", &sentinel));
    CHECK(fcntl(sentinel, F_GETFD) & FD_CLOEXEC);
    CHECK(!close(sentinel));
    CHECK(unlinkat(d, "fresh", 0) == 0);
    CHECK(!mkdirat(d, "inside", 0700));
    int inner = openat(d, "inside", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    CHECK(inner >= 0);
    file(inner, "item", "SAFE");
    CHECK(!symlinkat("inside", d, "middle"));
    CHECK(!fstatat(d, "middle/item", &before, 0));
    CHECK(!mkdirat(d, "replacement", 0700));
    int replacement =
        openat(d, "replacement", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    CHECK(replacement >= 0);
    file(replacement, "item", "EVIL");
    /* Deterministic changed-condition adversary transition between check/use. */
    CHECK(!unlinkat(d, "middle", 0) && !symlinkat("replacement", d, "middle"));
    int final_only =
        openat(d, "middle/item", O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    CHECK(final_only >= 0);
    content(final_only, "EVIL");
    CHECK(!fstat(final_only, &after) && before.st_ino != after.st_ino);
    CHECK(!close(final_only));
    sentinel = 77;
    CHECK(pn_open_confined(d, "middle/item", &sentinel) == ELOOP &&
          sentinel == 77);
    CHECK(!unlinkat(inner, "item", 0) && !close(inner) &&
          !unlinkat(d, "inside", AT_REMOVEDIR) && !unlinkat(d, "middle", 0));
    CHECK(!unlinkat(replacement, "item", 0) && !close(replacement) &&
          !unlinkat(d, "replacement", AT_REMOVEDIR));
    CHECK(!unlinkat(d, "chosen", 0) && !unlinkat(d, "safe", 0) &&
          !unlinkat(d, "other", 0) && !close(d) && !rmdir(root));
    puts("OBS race acknowledged_swap=EVIL retained_descriptor=SAFE "
         "exclusive=EEXIST intermediate_link=ELOOP");
}

static void policy(void)
{
    char root[512];
    temporary(root, "policy");
    int d = open(root, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    CHECK(d >= 0);
    file(d, "log", "SAFE");
    int fd = openat(d, "log", O_RDONLY | O_CLOEXEC);
    CHECK(fd >= 0);
    char out[4] = {'X', 'X', 'X', 'X'};
    CHECK(!sd_request(1, SD_READ_LOG, fd, out) && !memcmp(out, "SAFE", 4));
    CHECK(sd_request(1, SD_WRITE_CONFIG, fd, out) == EACCES);
    errno = 0;
    CHECK(write(fd, "X", 1) == -1 && errno == EBADF);
    content(fd, "SAFE");
    CHECK(lseek(fd, 0, SEEK_CUR) == 0);
    memcpy(out, "KEEP", 4);
    CHECK(sd_request(0, SD_READ_LOG, fd, out) == EACCES &&
          !memcmp(out, "KEEP", 4));
    CHECK(sd_request(0, SD_READ_LOG, -1, out) == EACCES &&
          !memcmp(out, "KEEP", 4));
    CHECK(lseek(fd, 0, SEEK_CUR) == 0);
    int reopened = openat(d, "log", O_WRONLY | O_CLOEXEC);
    CHECK(reopened >= 0 && pwrite(reopened, "OPEN", 4, 0) == 4);
    content(fd, "OPEN");
    CHECK(!close(reopened) && !close(fd) && !unlinkat(d, "log", 0) &&
          !close(d) && !rmdir(root));
    puts("OBS policy read_allowed=1 write_denied=EACCES readonly_write=EBADF "
         "revoked_read=no_effect ambient_reopen=observed");
}

static void spawned(const char *worker, int log, int tmp, int delimiter,
                    char *reopen)
{
    char *args[] = {"original-worker",
                    delimiter ? "--" : "--admin",
                    delimiter ? "--admin" : "a b",
                    "semi;colon",
                    "$(synthetic)",
                    NULL};
    char *env[] = {"SD_ONLY=yes", delimiter ? reopen : NULL, NULL};
    struct sd_child c;
    CHECK(!sd_spawn(worker, args, env, log, tmp, 0, &c));
    char text[2048];
    size_t n = 0;
    ssize_t r;
    while ((r = read(c.output, text + n, sizeof text - 1 - n)) > 0)
    {
        n += (size_t)r;
        CHECK(n < sizeof text - 1);
    }
    CHECK(r == 0);
    text[n] = 0;
    CHECK(!close(c.output));
    int status;
    CHECK(waitpid(c.pid, &status, 0) == c.pid && WIFEXITED(status));
    printf("OBS worker delimiter=%d %s", delimiter, text);
    CHECK(strstr(text, "inventory=PASS"));
    CHECK(WEXITSTATUS(status) == 0);
    CHECK(strstr(text, "FD 0\n") && strstr(text, "FD 4\n") &&
          !strstr(text, "FD 3\n") && strstr(text, "inventory=PASS"));
    CHECK(strstr(text, "ARG 10:semi;colon\n") &&
          strstr(text, "ARG 12:$(synthetic)\n"));
    CHECK(strstr(text, delimiter ? "admin=0" : "admin=1"));
    if (!delimiter)
        CHECK(strstr(text, "ARG 3:a b\n"));
    if (delimiter)
        CHECK(strstr(text, "REOPEN ambient_write=observed"));
}

static void inheritance(const char *worker)
{
    char root[512];
    temporary(root, "exec");
    int d = open(root, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    CHECK(d >= 0);
    file(d, "log", "SAFE");
    int log = openat(d, "log", O_RDONLY | O_CLOEXEC);
    int tmp =
        openat(d, "temporary", O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    CHECK(log >= 0 && tmp >= 0);
    unsigned initial = descriptors();
    char reopen[768], missing[768];
    CHECK(snprintf(reopen, sizeof reopen, "SD_REOPEN=%s/log", root) > 0);
    CHECK(snprintf(missing, sizeof missing, "%s/missing-worker", root) > 0);
    for (unsigned f = 1; f <= 4; ++f)
    {
        char *a[] = {"worker", NULL}, *e[] = {NULL};
        struct sd_child out = {-7, -8};
        CHECK(sd_spawn(worker, a, e, log, tmp, f, &out) == EIO &&
              out.pid == -7 && out.output == -8);
        CHECK(descriptors() == initial);
    }
    {
        char *a[] = {"worker", NULL}, *e[] = {NULL};
        struct sd_child out = {-7, -8};
        CHECK(sd_spawn(missing, a, e, log, tmp, 0, &out) == ENOENT &&
              out.pid == -7 && out.output == -8);
        CHECK(descriptors() == initial);
    }
    int high = fcntl(log, F_DUPFD, 900);
    CHECK(high >= 900);
    spawned(worker, log, tmp, 0, reopen);
    spawned(worker, log, tmp, 1, reopen);
    CHECK(!close(high));
    CHECK(descriptors() == initial);
    content(log, "OPEN");
    CHECK(!close(log) && !close(tmp) && !unlinkat(d, "temporary", 0) &&
          !unlinkat(d, "log", 0) && !close(d) && !rmdir(root));
    puts(
        "OBS inheritance fixture_fds=0,4 closed=3 faults=4 exec_failure=ENOENT "
        "cleanup=exact");
}

int main(int argc, char **argv)
{
    CHECK(argc >= 2);
    unsigned before = descriptors();
    if (!strcmp(argv[1], "race"))
        race();
    else if (!strcmp(argv[1], "policy"))
        policy();
    else if (!strcmp(argv[1], "inheritance"))
    {
        CHECK(argc == 3);
        inheritance(argv[2]);
    }
    else
        CHECK(0);
    CHECK(descriptors() == before);
    return 0;
}
