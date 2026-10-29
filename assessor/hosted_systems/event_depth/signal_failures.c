#include "signals.h"
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
int __real_pipe2(int *, int);
int __real_sigaction(int, const struct sigaction *, struct sigaction *);
static unsigned stage, acquisition;

int __wrap_pipe2(int *fds, int flags)
{
    if (++acquisition == stage)
    {
        errno = EMFILE;
        return -1;
    }
    return __real_pipe2(fds, flags);
}

int __wrap_sigaction(int sig, const struct sigaction *a, struct sigaction *old)
{
    if (a && ++acquisition == stage)
    {
        errno = EINVAL;
        return -1;
    }
    return __real_sigaction(sig, a, old);
}

static unsigned descriptors(void)
{
    DIR *d = opendir("/proc/self/fd");
    if (!d)
    {
        return 99999;
    }
    unsigned n = 0;
    struct dirent *entry;
    while ((entry = readdir(d)))
    {
        if (entry->d_name[0] != '.')
        {
            ++n;
        }
    }
    closedir(d);
    return n;
}

#define CHECK(c)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(c))                                                          \
        {                                                                  \
            fprintf(stderr, "SIGNAL-FAILURE-CHECK:%s:%d\n", #c, __LINE__); \
            return 1;                                                      \
        }                                                                  \
    } while (0)

int main(void)
{
    const int sigs[] = {SIGUSR1, SIGUSR2, SIGCHLD};
    for (unsigned cut = 1; cut <= 4; ++cut)
    {
        sigset_t mask, after;
        struct sigaction saved[3], got;
        CHECK(pthread_sigmask(SIG_SETMASK, 0, &mask) == 0);
        for (unsigned i = 0; i < 3; ++i)
        {
            CHECK(sigaction(sigs[i], 0, &saved[i]) == 0);
        }
        unsigned before = descriptors();
        CHECK(before != 99999);
        stage = cut;
        acquisition = 0;
        struct es_signals p = {0};
        CHECK(es_open(&p) == -1 && !p.active);
        CHECK(errno == (cut == 1 ? EMFILE : EINVAL));
        CHECK(descriptors() == before);
        CHECK(pthread_sigmask(SIG_SETMASK, 0, &after) == 0);
        for (int i = 1; i < NSIG; ++i)
        {
            CHECK(sigismember(&mask, i) == sigismember(&after, i));
        }
        for (unsigned i = 0; i < 3; ++i)
        {
            CHECK(sigaction(sigs[i], 0, &got) == 0);
            CHECK(got.sa_handler == saved[i].sa_handler);
            for (int j = 1; j < NSIG; ++j)
            {
                CHECK(sigismember(&got.sa_mask, j) ==
                      sigismember(&saved[i].sa_mask, j));
            }
        }
        stage = 0;
        CHECK(es_open(&p) == 0 && es_close(&p) == 0);
        CHECK(descriptors() == before);
    }
    puts("SIGNAL-FAILURES:acquisition_cuts=4;descriptor_balance=4;dispositions_"
         "restored=12;masks_restored=4;retry_success=4;");
    return 0;
}
