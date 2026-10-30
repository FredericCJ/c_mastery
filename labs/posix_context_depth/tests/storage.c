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

enum
{
    PATH_BYTES = 4096,
    FILE_BYTES = 32,
    BUFFER_BYTES = 64,
    LAST_STAGE = 4,
    /* First stage after which the new contents are visible (the rename). */
    FIRST_VISIBLE_STAGE = 3,
    CUT_REPORT_FAILED_STATUS = 91,
    CUT_RELEASE_RETURNED_STATUS = 92,
    CHILD_REPLACE_FAILED_STATUS = 93,
    CHILD_REPLACE_RETURNED_STATUS = 94
};

/* Installs the "old" generation of the target file, durably. */
static void install_old(int dir)
{
    int fd =
        openat(dir, "data", O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    CHECK(fd >= 0 && write(fd, "old", 3) == 3 && fsync(fd) == 0 &&
          close(fd) == 0 && fsync(dir) == 0);
}

static void contents(int dir, const char *name, const char *expected)
{
    char buffer[FILE_BYTES] = {0};
    int fd = openat(dir, name, O_RDONLY | O_CLOEXEC);
    CHECK(fd >= 0);
    ssize_t got = read(fd, buffer, sizeof buffer);
    CHECK(got == (ssize_t)strlen(expected) &&
          !memcmp(buffer, expected, (size_t)got));
    CHECK(close(fd) == 0);
}

/* Step hook that proves stages are reported once, in order, without gaps. */
static void sequence(void *context, unsigned stage)
{
    unsigned *previous = context;
    CHECK(stage == *previous + 1);
    *previous = stage;
}

/* Process-cut protocol: at stage `cut` the child tells the parent over
 * `report` and then blocks reading `wait`, so the parent can SIGKILL it at a
 * known point. */
struct pc_cut
{
    int report;
    int wait;
    unsigned cut;
};

static void stop_after(void *context, unsigned stage)
{
    struct pc_cut *cut = context;
    if (stage != cut->cut)
    {
        return;
    }
    unsigned char token = (unsigned char)stage;
    ssize_t n;
    do
    {
        n = write(cut->report, &token, 1);
    } while (n < 0 && errno == EINTR);
    if (n != 1)
    {
        _exit(CUT_REPORT_FAILED_STATUS);
    }
    do
    {
        n = read(cut->wait, &token, 1);
    } while (n < 0 && errno == EINTR);
    _exit(CUT_RELEASE_RETURNED_STATUS);
}

/* Child body of cuts(): runs pc_replace and never returns. */
static void replace_until_cut(int dir, int lock, struct pc_cut *cut)
{
    struct pc_replace result;
    int error = pc_replace(dir, lock, (const unsigned char *)"new", 3, 0,
                           stop_after, cut, &result);
    _exit(error ? CHILD_REPLACE_FAILED_STATUS : CHILD_REPLACE_RETURNED_STATUS);
}

/* Kills a real process after each stage. Stages 1-2 leave the old contents
 * visible and a stray temporary; stages 3-4 expose the new contents. Whether
 * the data is durable is NOT observed (process death is not power loss). */
static void cuts(int dir, int lock)
{
    for (unsigned cut = 1; cut <= LAST_STAGE; ++cut)
    {
        int report[2];
        int wait_for_release[2];
        int status;
        unsigned char stage;
        install_old(dir);
        CHECK(pipe2(report, O_CLOEXEC) == 0 &&
              pipe2(wait_for_release, O_CLOEXEC) == 0);
        pid_t pid = fork();
        CHECK(pid >= 0);
        if (!pid)
        {
            close(report[0]);
            close(wait_for_release[1]);
            struct pc_cut child_cut = {report[1], wait_for_release[0], cut};
            replace_until_cut(dir, lock, &child_cut);
        }
        close(report[1]);
        close(wait_for_release[0]);
        CHECK(read(report[0], &stage, 1) == 1 && stage == cut);
        CHECK(kill(pid, SIGKILL) == 0 && waitpid(pid, &status, 0) == pid &&
              WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL);
        close(report[0]);
        close(wait_for_release[1]);
        /* flock() locks belong to the open file description, which the
         * parent still references: explicitly release its surviving share. */
        CHECK(flock(lock, LOCK_UN) == 0);
        contents(dir, "data", cut < FIRST_VISIBLE_STAGE ? "old" : "new");
        if (cut < FIRST_VISIBLE_STAGE)
        {
            CHECK(unlinkat(dir, "temporary", 0) == 0);
        }
        printf("PC_PROCESS_CUT stage=%u visible=%s persistence=unobserved\n",
               cut, cut < FIRST_VISIBLE_STAGE ? "old" : "new");
    }
}

/* flock() is advisory and per inode: the replacement scheme must lock a
 * stable file ("lock"), never the replaced "data" inode. */
static void locking(int dir, int stable)
{
    install_old(dir);
    int first = openat(dir, "data", O_RDWR | O_CLOEXEC);
    CHECK(first >= 0 && flock(first, LOCK_EX | LOCK_NB) == 0);
    struct pc_replace result;
    CHECK(pc_replace(dir, stable, (const unsigned char *)"new", 3, 0, NULL,
                     NULL, &result) == 0);
    int second_open = openat(dir, "data", O_RDWR | O_CLOEXEC);
    struct stat old_inode;
    struct stat new_inode;
    CHECK(second_open >= 0 && fstat(first, &old_inode) == 0 &&
          fstat(second_open, &new_inode) == 0 &&
          old_inode.st_ino != new_inode.st_ino);
    /* Different inode: both claims of exclusive pathname ownership coexist. */
    CHECK(flock(second_open, LOCK_EX | LOCK_NB) == 0);
    CHECK(flock(first, LOCK_UN) == 0 && flock(second_open, LOCK_UN) == 0 &&
          close(first) == 0 && close(second_open) == 0);
    CHECK(flock(stable, LOCK_EX | LOCK_NB) == 0);
    int contender = openat(dir, "lock", O_RDWR | O_CLOEXEC);
    CHECK(contender >= 0 && flock(contender, LOCK_EX | LOCK_NB) == -1 &&
          (errno == EWOULDBLOCK || errno == EAGAIN));
    /* Synthetic uncooperative writer: advisory ownership alone does not stop I/O. */
    int rogue = openat(dir, "data", O_WRONLY | O_TRUNC | O_CLOEXEC);
    CHECK(rogue >= 0 && write(rogue, "rogue", 5) == 5 && close(rogue) == 0);
    contents(dir, "data", "rogue");
    CHECK(flock(stable, LOCK_UN) == 0 &&
          flock(contender, LOCK_EX | LOCK_NB) == 0 &&
          flock(contender, LOCK_UN) == 0 && close(contender) == 0);
    puts(
        "PC_LOCK stable_conflict=observed replaced_inode_split=observed uncooperative_write=observed");
}

/* Application buffer, kernel page cache and stable storage are three
 * different places; fsync() applies only to files, not pipes. */
static void buffering(int dir)
{
    int fd =
        openat(dir, "buffered", O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    CHECK(fd >= 0);
    FILE *stream = fdopen(fd, "w");
    CHECK(stream);
    char buffer[BUFFER_BYTES];
    CHECK(setvbuf(stream, buffer, _IOFBF, sizeof buffer) == 0);
    CHECK(fputc('x', stream) == 'x');
    struct stat info;
    CHECK(fstat(fd, &info) == 0 && info.st_size == 0);
    CHECK(fflush(stream) == 0 && fstat(fd, &info) == 0 && info.st_size == 1);
    CHECK(fsync(fd) == 0 && fclose(stream) == 0);
    CHECK(unlinkat(dir, "buffered", 0) == 0);
    int channel[2];
    CHECK(pipe(channel) == 0);
    CHECK(fsync(channel[1]) == -1 && errno == EINVAL);
    close(channel[0]);
    close(channel[1]);
    puts(
        "PC_BUFFER app=0 kernel_visible=1 file_sync=accepted pipe_sync=EINVAL physical=unobserved");
}

/* Injects each failure stage 0..4 and checks the contract: stage reached,
 * which generation is visible, and no leftover temporary. */
static void failure_matrix(int dir, int lock)
{
    for (unsigned failure = 0; failure <= LAST_STAGE; ++failure)
    {
        unsigned stage = 0;
        install_old(dir);
        struct pc_replace result = {99, 99, 99};
        int error = pc_replace(dir, lock, (const unsigned char *)"new", 3,
                               failure, sequence, &stage, &result);
        CHECK(error == (failure ? EIO : 0));
        CHECK(result.error == error &&
              result.stage == (failure ? failure - 1 : LAST_STAGE) &&
              result.visible == (failure == 0 || failure == LAST_STAGE));
        contents(dir, "data", result.visible ? "new" : "old");
        CHECK(faccessat(dir, "temporary", F_OK, 0) == -1 && errno == ENOENT);
        printf("PC_FAILURE stage=%u completed=%u error=%d visible=%d\n",
               failure, result.stage, error, result.visible);
    }
}

/* A "temporary" that someone else created must be neither used nor removed. */
static void foreign_temporary(int dir, int lock)
{
    install_old(dir);
    int fd =
        openat(dir, "temporary", O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    CHECK(fd >= 0 && write(fd, "owned elsewhere", 15) == 15 && close(fd) == 0);
    struct pc_replace result;
    CHECK(pc_replace(dir, lock, (const unsigned char *)"new", 3, 0, NULL, NULL,
                     &result) == EEXIST);
    contents(dir, "temporary", "owned elsewhere");
    CHECK(unlinkat(dir, "temporary", 0) == 0);
}

int main(void)
{
    char name[PATH_BYTES];
    const char *root = getenv("PC_STORAGE_ROOT");
    CHECK(root && root[0] == '/');
    int length = snprintf(name, sizeof name, "%s/storage-XXXXXX", root);
    CHECK(length > 0 && (size_t)length < sizeof name && mkdtemp(name));
    int dir = open(name, O_DIRECTORY | O_RDONLY | O_CLOEXEC);
    CHECK(dir >= 0);
    int lock = openat(dir, "lock", O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    CHECK(lock >= 0);
    struct statfs filesystem;
    CHECK(fstatfs(dir, &filesystem) == 0);
    printf("PC_FILESYSTEM type=%lx private_directory=%s\n",
           (unsigned long)filesystem.f_type, name);
    failure_matrix(dir, lock);
    foreign_temporary(dir, lock);
    struct pc_replace result;
    install_old(dir);
    CHECK(pc_replace(dir, lock, NULL, 0, 0, NULL, NULL, &result) == 0);
    contents(dir, "data", "");
    buffering(dir);
    locking(dir, lock);
    cuts(dir, lock);
    CHECK(unlinkat(dir, "data", 0) == 0 && unlinkat(dir, "lock", 0) == 0 &&
          close(lock) == 0 && close(dir) == 0 && rmdir(name) == 0);
    puts("PC_STORAGE_PASS");
    return 0;
}
