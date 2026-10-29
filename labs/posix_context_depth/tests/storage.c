#define _GNU_SOURCE
#include "context.h"
#include "test.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/wait.h>
#include <unistd.h>

static void old(int dir)
{
    int f = openat(dir, "data", O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    CHECK(f >= 0 && write(f, "old", 3) == 3 && fsync(f) == 0 && close(f) == 0 &&
          fsync(dir) == 0);
}

static void contents(int dir, const char *name, const char *expected)
{
    char b[32] = {0};
    int f = openat(dir, name, O_RDONLY | O_CLOEXEC);
    CHECK(f >= 0);
    ssize_t n = read(f, b, sizeof b);
    CHECK(n == (ssize_t)strlen(expected) && !memcmp(b, expected, (size_t)n));
    CHECK(close(f) == 0);
}

static void sequence(void *p, unsigned stage)
{
    unsigned *previous = p;
    CHECK(stage == *previous + 1);
    *previous = stage;
}

struct pc_cut
{
    int report, wait;
    unsigned cut;
};

static void stop_after(void *p, unsigned stage)
{
    struct pc_cut *c = p;
    if (stage == c->cut)
    {
        unsigned char x = (unsigned char)stage;
        ssize_t n;
        do
        {
            n = write(c->report, &x, 1);
        } while (n < 0 && errno == EINTR);
        if (n != 1)
            _exit(91);
        do
        {
            n = read(c->wait, &x, 1);
        } while (n < 0 && errno == EINTR);
        _exit(92);
    }
}

static void cuts(int dir, int lock)
{
    for (unsigned cut = 1; cut <= 4; ++cut)
    {
        int p[2], q[2], status;
        unsigned char stage;
        old(dir);
        CHECK(pipe2(p, O_CLOEXEC) == 0 && pipe2(q, O_CLOEXEC) == 0);
        pid_t pid = fork();
        CHECK(pid >= 0);
        if (!pid)
        {
            close(p[0]);
            close(q[1]);
            struct pc_cut c = {p[1], q[0], cut};
            struct pc_replace r;
            int e = pc_replace(dir, lock, (const unsigned char *)"new", 3, 0,
                               stop_after, &c, &r);
            _exit(e ? 93 : 94);
        }
        close(p[1]);
        close(q[0]);
        CHECK(read(p[0], &stage, 1) == 1 && stage == cut);
        CHECK(kill(pid, SIGKILL) == 0 && waitpid(pid, &status, 0) == pid &&
              WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL);
        close(p[0]);
        close(
            q[1]); /* Parent owns the same lock description; explicitly release its surviving reference. */
        CHECK(flock(lock, LOCK_UN) == 0);
        contents(dir, "data", cut < 3 ? "old" : "new");
        if (cut < 3)
            CHECK(unlinkat(dir, "temporary", 0) == 0);
        printf("PC_PROCESS_CUT stage=%u visible=%s persistence=unobserved\n",
               cut, cut < 3 ? "old" : "new");
    }
}

static void locking(int dir, int stable)
{
    old(dir);
    int a = openat(dir, "data", O_RDWR | O_CLOEXEC);
    CHECK(a >= 0 && flock(a, LOCK_EX | LOCK_NB) == 0);
    struct pc_replace r;
    CHECK(pc_replace(dir, stable, (const unsigned char *)"new", 3, 0, NULL,
                     NULL, &r) == 0);
    int b = openat(dir, "data", O_RDWR | O_CLOEXEC);
    struct stat x, y;
    CHECK(b >= 0 && fstat(a, &x) == 0 && fstat(b, &y) == 0 &&
          x.st_ino != y.st_ino);
    CHECK(
        flock(b, LOCK_EX | LOCK_NB) ==
        0); /* Different inode: both claims of exclusive pathname ownership coexist. */
    CHECK(flock(a, LOCK_UN) == 0 && flock(b, LOCK_UN) == 0 && close(a) == 0 &&
          close(b) == 0);
    CHECK(flock(stable, LOCK_EX | LOCK_NB) == 0);
    int second = openat(dir, "lock", O_RDWR | O_CLOEXEC);
    CHECK(second >= 0 && flock(second, LOCK_EX | LOCK_NB) == -1 &&
          (errno == EWOULDBLOCK || errno == EAGAIN));
    /* Synthetic uncooperative writer: advisory ownership alone does not stop I/O. */
    int f = openat(dir, "data", O_WRONLY | O_TRUNC | O_CLOEXEC);
    CHECK(f >= 0 && write(f, "rogue", 5) == 5 && close(f) == 0);
    contents(dir, "data", "rogue");
    CHECK(flock(stable, LOCK_UN) == 0 &&
          flock(second, LOCK_EX | LOCK_NB) == 0 &&
          flock(second, LOCK_UN) == 0 && close(second) == 0);
    puts(
        "PC_LOCK stable_conflict=observed replaced_inode_split=observed uncooperative_write=observed");
}

static void buffering(int dir)
{
    int f =
        openat(dir, "buffered", O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    CHECK(f >= 0);
    FILE *s = fdopen(f, "w");
    CHECK(s);
    char buffer[64];
    CHECK(setvbuf(s, buffer, _IOFBF, sizeof buffer) == 0);
    CHECK(fputc('x', s) == 'x');
    struct stat st;
    CHECK(fstat(f, &st) == 0 && st.st_size == 0);
    CHECK(fflush(s) == 0 && fstat(f, &st) == 0 && st.st_size == 1);
    CHECK(fsync(f) == 0 && fclose(s) == 0);
    CHECK(unlinkat(dir, "buffered", 0) == 0);
    int p[2];
    CHECK(pipe(p) == 0);
    CHECK(fsync(p[1]) == -1 && errno == EINVAL);
    close(p[0]);
    close(p[1]);
    puts(
        "PC_BUFFER app=0 kernel_visible=1 file_sync=accepted pipe_sync=EINVAL physical=unobserved");
}

int main(void)
{
    char name[4096];
    const char *root = getenv("PC_STORAGE_ROOT");
    CHECK(root && root[0] == '/');
    int length = snprintf(name, sizeof name, "%s/storage-XXXXXX", root);
    CHECK(length > 0 && (size_t)length < sizeof name && mkdtemp(name));
    int dir = open(name, O_DIRECTORY | O_RDONLY | O_CLOEXEC);
    CHECK(dir >= 0);
    int lock = openat(dir, "lock", O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    CHECK(lock >= 0);
    struct statfs fs;
    CHECK(fstatfs(dir, &fs) == 0);
    printf("PC_FILESYSTEM type=%lx private_directory=%s\n",
           (unsigned long)fs.f_type, name);
    for (unsigned failure = 0; failure <= 4; ++failure)
    {
        unsigned stage = 0;
        old(dir);
        struct pc_replace r = {99, 99, 99};
        int e = pc_replace(dir, lock, (const unsigned char *)"new", 3, failure,
                           sequence, &stage, &r);
        CHECK(e == (failure ? EIO : 0));
        CHECK(r.error == e && r.stage == (failure ? failure - 1 : 4) &&
              r.visible == (failure == 0 || failure == 4));
        contents(dir, "data", r.visible ? "new" : "old");
        CHECK(faccessat(dir, "temporary", F_OK, 0) == -1 && errno == ENOENT);
        printf("PC_FAILURE stage=%u completed=%u error=%d visible=%d\n",
               failure, r.stage, e, r.visible);
    }
    old(dir);
    int f =
        openat(dir, "temporary", O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    CHECK(f >= 0 && write(f, "owned elsewhere", 15) == 15 && close(f) == 0);
    struct pc_replace r;
    CHECK(pc_replace(dir, lock, (const unsigned char *)"new", 3, 0, NULL, NULL,
                     &r) == EEXIST);
    contents(dir, "temporary", "owned elsewhere");
    CHECK(unlinkat(dir, "temporary", 0) == 0);
    old(dir);
    CHECK(pc_replace(dir, lock, NULL, 0, 0, NULL, NULL, &r) == 0);
    contents(dir, "data", "");
    buffering(dir);
    locking(dir, lock);
    cuts(dir, lock);
    CHECK(unlinkat(dir, "data", 0) == 0 && unlinkat(dir, "lock", 0) == 0 &&
          close(lock) == 0 && close(dir) == 0 && rmdir(name) == 0);
    puts("PC_STORAGE_PASS");
    return 0;
}
