#define _GNU_SOURCE
#include "oslab.h"
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "OF_LIFE:%d:%s:errno%d\n", __LINE__, #x, errno); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

struct of_arg
{
    atomic_uint *counter;
    unsigned id;
};

static void *worker(void *v)
{
    struct of_arg *a = v;
    for (unsigned i = 0; i < 1000; i++)
    {
        atomic_fetch_add_explicit(a->counter, 1, memory_order_relaxed);
    }
    pthread_exit(a);
}

static int threads(unsigned fail)
{
    atomic_uint count;
    atomic_init(&count, 0);
    pthread_t owned[2];
    struct of_arg args[2] = {{&count, 0}, {&count, 1}};
    unsigned made = 0, joined = 0;
    int first_error = 0;
    for (; made < 2; made++)
    {
        int result = fail && made == 1 ? EAGAIN
                                       : pthread_create(&owned[made], NULL,
                                                        worker, &args[made]);
        if (result)
        {
            first_error = result;
            break;
        }
    }
    for (unsigned i = 0; i < made; i++)
    {
        void *result = NULL;
        CHECK(pthread_join(owned[i], &result) == 0 && result == &args[i]);
        joined++;
    }
    CHECK(made == joined && atomic_load(&count) == made * 1000 &&
          made == (fail ? 1u : 2u));
    CHECK(first_error == (fail ? EAGAIN : 0));
    printf(
        "OF:threads:made%u:joined%u:count%u:scoped-args-live:direct-error%d\n",
        made, joined, atomic_load(&count), first_error);
    return 0;
}

struct of_shared
{
    sem_t ready;
    unsigned published, value;
};

static int process(unsigned fail, unsigned shared_result)
{
    struct of_shared *shared =
        mmap(NULL, sizeof *shared, PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    CHECK(shared != MAP_FAILED);
    memset(shared, 0, sizeof *shared);
    CHECK(sem_init(&shared->ready, 1, 0) == 0);
    int pipefd[2];
    CHECK(pipe(pipefd) == 0);
    int private = 7;
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        close(pipefd[0]);
      private
        = 9;
        if (fail)
        {
            close(pipefd[1]);
            _exit(7);
        }
        shared->value = 42;
        shared->published = 1;
        if (shared_result)
        {
            if (sem_post(&shared->ready))
            {
                _exit(8);
            }
        }
        else if (write(pipefd[1], &private, sizeof private) !=
                 (ssize_t)sizeof private)
        {
            _exit(9);
        }
        close(pipefd[1]);
        _exit(0);
    }
    CHECK(close(pipefd[1]) == 0);
    int received = 0;
    ssize_t bytes = 0;
    if (!shared_result)
    {
        bytes = read(pipefd[0], &received, sizeof received);
    }
    else if (!fail)
    {
        int rc;
        do
        {
            rc = sem_wait(&shared->ready);
        } while (rc && errno == EINTR);
        CHECK(!rc && shared->published && shared->value == 42);
    }
    CHECK(close(pipefd[0]) == 0);
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status));
    CHECK(private == 7 && WEXITSTATUS(status) == (fail ? 7 : 0));
    if (fail)
    {
        CHECK(!shared->published && (!shared_result ? bytes == 0 : 1));
    }
    else
    {
        CHECK(shared->value == 42 &&
              (shared_result
                   ? shared->published
                   : bytes == (ssize_t)sizeof private && received == 9));
    }
    printf(
        "OF:process:private7:child%s:shared%u:publication%u:channel%u:status%"
        "d:reaped\n",
        fail ? "failed-before-report" : "private9", shared->value,
        shared->published, shared_result, WEXITSTATUS(status));
    CHECK(sem_destroy(&shared->ready) == 0 &&
          munmap(shared, sizeof *shared) == 0);
    return 0;
}

static int buffering(unsigned fixed)
{
    FILE *file = tmpfile();
    CHECK(file);
    char buffer[256];
    CHECK(setvbuf(file, buffer, _IOFBF, sizeof buffer) == 0);
    CHECK(fwrite("prefix", 1, 6, file) == 6);
    if (fixed)
    {
        CHECK(fflush(file) == 0);
    }
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        exit(0);
    }
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          !WEXITSTATUS(status));
    CHECK(fflush(file) == 0 && fseek(file, 0, SEEK_SET) == 0);
    char text[32] = {0};
    size_t n = fread(text, 1, sizeof text, file);
    CHECK(n == (fixed ? 6u : 12u) &&
          !memcmp(text, fixed ? "prefix" : "prefixprefix", n));
    CHECK(fclose(file) == 0);
    printf("OF:stdio:pre-fork-flush%u:bytes%zu:owned-child-exit\n", fixed, n);
    return 0;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    CHECK(os_process_probe() == 0 && os_thread_probe() == 0);
    if (threads(0) || threads(1) || process(0, 0) || process(1, 0) ||
        process(0, 1) || process(1, 1) || buffering(0) || buffering(1))
    {
        return 1;
    }
    int status;
    CHECK(waitpid(-1, &status, WNOHANG) == -1 && errno == ECHILD);
    puts("OF_PASS:lifecycle:no-owned-child-left");
    return 0;
}
