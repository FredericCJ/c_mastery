#include "common.h"

int hs_signal_roundtrip(int sig)
{
    if (sig != SIGUSR1 && sig != SIGUSR2)
        return -1;
    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, sig);
    if (pthread_sigmask(SIG_BLOCK, &set, &old))
        return -1;
    int result = -1;
    if (!kill(getpid(), sig))
    {
        struct timespec timeout = {1, 0};
        int r;
        do
        {
            r = sigtimedwait(&set, 0, &timeout);
        } while (r < 0 && errno == EINTR);
        if (r == sig)
            result = 0;
    }
    if (pthread_sigmask(SIG_SETMASK, &old, 0))
        return -1;
    return result;
}
