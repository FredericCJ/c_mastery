#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(x)                                              \
    do                                                        \
    {                                                         \
        if (!(x))                                             \
        {                                                     \
            fprintf(stderr, "DX_HOST:%d:%s\n", __LINE__, #x); \
            exit(1);                                          \
        }                                                     \
    } while (0)

static void pipe_case(int ignored)
{
    int f[2];
    CHECK(pipe(f) == 0);
    pid_t p = fork();
    CHECK(p >= 0);
    if (!p)
    {
        close(f[0]);
        if (signal(SIGPIPE, ignored ? SIG_IGN : SIG_DFL) == SIG_ERR)
        {
            _exit(4);
        }
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGPIPE);
        if (pthread_sigmask(SIG_UNBLOCK, &set, NULL))
        {
            _exit(5);
        }
        errno = 0;
        ssize_t n = write(f[1], "x", 1);
        _exit(ignored && n == -1 && errno == EPIPE ? 0 : 6);
    }
    close(f[0]);
    close(f[1]);
    int status;
    CHECK(waitpid(p, &status, 0) == p);
    CHECK(ignored ? (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                  : (WIFSIGNALED(status) && WTERMSIG(status) == SIGPIPE));
    printf("DX:SIGPIPE:%s:%d\n", ignored ? "ignored-EPIPE" : "default-signal",
           status);
}

int main(void)
{
    errno = ENOSPC;
    int saved = errno;
    errno = EIO;
    CHECK(saved == ENOSPC && errno == EIO);
    unsigned descriptors = 2, acquired = 0;
    int failure = 0;
    for (unsigned i = 0; i < 7; i++)
    {
        if (descriptors == 8)
        {
            failure = EMFILE;
            break;
        }
        descriptors++;
        acquired++;
    }
    CHECK(acquired == 6 && failure == EMFILE);
    while (acquired)
    {
        acquired--;
        descriptors--;
    }
    CHECK(descriptors == 2);
    pthread_mutexattr_t attr;
    CHECK(pthread_mutexattr_init(&attr) == 0);
    errno = ENOSPC;
    int direct = pthread_mutexattr_settype(&attr, -100);
    CHECK(direct == EINVAL && errno == ENOSPC);
    CHECK(pthread_mutexattr_destroy(&attr) == 0);
    puts(
        "DX:errno:ENOSPC-survives-EIO:ledger6-of7:EMFILE:direct-pthread-EINVAL");
    pipe_case(0);
    pipe_case(1);
    return 0;
}
