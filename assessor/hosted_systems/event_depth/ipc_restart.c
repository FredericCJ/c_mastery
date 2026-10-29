#include "ipc.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

/* Same-executable, same-host ABI object; not a persistent portable mutex
 * format. Restart only after all previous participants have exited and been
 * joined. */
struct rd_record
{
    uint32_t magic, layout, epoch, request_epoch, state;
    pthread_mutex_t mutex;
    unsigned char request[8], response[4];
};

#define MAGIC UINT32_C(0x434d4950)
#define CHECK(c)                                                      \
    do                                                                \
    {                                                                 \
        if (!(c))                                                     \
        {                                                             \
            fprintf(stderr, "IPC-RESTART-CHECK:%s:%d errno=%d\n", #c, \
                    __LINE__, errno);                                 \
            return 1;                                                 \
        }                                                             \
    } while (0)

static int serve(const char *name, unsigned epoch, int forbidden)
{
    /* Check before opening a new descriptor: private pipe endpoint was CLOEXEC.
     */
    CHECK(fcntl(forbidden, F_GETFD) == -1 && errno == EBADF);
    int fd = shm_open(name, O_RDWR, 0);
    CHECK(fd >= 0);
    struct stat st;
    CHECK(fstat(fd, &st) == 0 && st.st_size == (off_t)sizeof(struct rd_record));
    struct rd_record *r =
        mmap(0, sizeof *r, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    CHECK(r != MAP_FAILED && close(fd) == 0 && r->magic == MAGIC &&
          r->layout == 1 && r->epoch == epoch);
    CHECK(pthread_mutex_lock(&r->mutex) == 0 && r->state == 1);
    if (r->request_epoch != epoch)
    {
        r->state = 3;
    }
    else
    {
        ei_reply(r->request, r->response);
        r->state = 2;
    }
    CHECK(pthread_mutex_unlock(&r->mutex) == 0 && munmap(r, sizeof *r) == 0);
    return 0;
}

static int start(const char *program, const char *name, unsigned epoch,
                 int forbidden)
{
    char e[16], f[16];
    snprintf(e, sizeof e, "%u", epoch);
    snprintf(f, sizeof f, "%d", forbidden);
    pid_t child = fork();
    if (child < 0)
    {
        return -1;
    }
    if (!child)
    {
        execl(program, program, "serve", name, e, f, (char *)0);
        _exit(127);
    }
    int status;
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0)
    {
        return -1;
    }
    return 0;
}

static int initialize_mutex(struct rd_record *r)
{
    pthread_mutexattr_t a;
    int result = pthread_mutexattr_init(&a);
    if (result)
    {
        return result;
    }
    result = pthread_mutexattr_setpshared(&a, PTHREAD_PROCESS_SHARED);
    if (!result)
    {
        result = pthread_mutex_init(&r->mutex, &a);
    }
    int cleanup = pthread_mutexattr_destroy(&a);
    return result ? result : cleanup;
}

int main(int argc, char **argv)
{
    alarm(10);
    if (argc == 5 && !strcmp(argv[1], "serve"))
    {
        return serve(argv[2], (unsigned)strtoul(argv[3], 0, 10), atoi(argv[4]));
    }
    CHECK(argc == 1);
    char name[64];
    snprintf(name, sizeof name, "/cmastery-original-ipc-%ld", (long)getpid());
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    CHECK(fd >= 0 && ftruncate(fd, sizeof(struct rd_record)) == 0);
    struct rd_record *r =
        mmap(0, sizeof *r, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    CHECK(r != MAP_FAILED && close(fd) == 0);
    memset(r, 0, sizeof *r);
    r->magic = MAGIC;
    r->layout = 1;
    r->epoch = 1;
    r->request_epoch = 1;
    r->state = 1;
    const unsigned char input[8] = {0, 1, 2, 3, 128, 254, 255, 0},
                        expected[4] = {0, 0, 2, 131};
    memcpy(r->request, input, 8);
    CHECK(initialize_mutex(r) == 0);
    int pipefd[2];
    CHECK(pipe2(pipefd, O_CLOEXEC) == 0);
    int forbidden = fcntl(pipefd[0], F_DUPFD_CLOEXEC, 200);
    CHECK(forbidden >= 200);
    close(pipefd[0]);
    close(pipefd[1]);
    CHECK(start(argv[0], name, 1, forbidden) == 0);
    CHECK(pthread_mutex_lock(&r->mutex) == 0 && r->state == 2 &&
          !memcmp(r->response, expected, 4));
    CHECK(pthread_mutex_unlock(&r->mutex) == 0);
    /* No live borrower remains. Old named data survived exec worker termination;
     * old mutex can now be destroyed, not blindly reinitialized while owned. */
    CHECK(pthread_mutex_destroy(&r->mutex) == 0);
    r->epoch = 2;
    r->state = 1;
    memset(r->response, 99, 4);
    CHECK(initialize_mutex(r) == 0);
    CHECK(start(argv[0], name, 2, forbidden) == 0);
    CHECK(pthread_mutex_lock(&r->mutex) == 0 && r->state == 3 &&
          r->response[0] == 99);
    r->request_epoch = 2;
    r->state = 1;
    CHECK(pthread_mutex_unlock(&r->mutex) == 0);
    CHECK(start(argv[0], name, 2, forbidden) == 0);
    CHECK(pthread_mutex_lock(&r->mutex) == 0 && r->state == 2 &&
          !memcmp(r->response, expected, 4));
    CHECK(pthread_mutex_unlock(&r->mutex) == 0);
    /* Removal prevents new attachment while existing mapping remains valid. */
    CHECK(shm_unlink(name) == 0);
    errno = 0;
    CHECK(shm_open(name, O_RDWR, 0) == -1 && errno == ENOENT && r->epoch == 2);
    CHECK(pthread_mutex_destroy(&r->mutex) == 0 && munmap(r, sizeof *r) == 0 &&
          close(forbidden) == 0);
    puts("IPC-RESTART:independent_execs=3;epochs=2;stale_request_rejected=1;"
         "named_data_survives_exit=1;cloexec_no_implicit_reconnect=1;quiescent_"
         "mutex_reinit=1;power_loss_durability=0;");
    return 0;
}
