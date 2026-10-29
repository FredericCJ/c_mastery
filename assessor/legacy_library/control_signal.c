#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
static volatile sig_atomic_t seen;

static void mark(int signal)
{
    (void)signal;
    seen = 1;
}

int main(void)
{
    sigset_t blocked, old;
    struct sigaction action = {0}, previous;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &blocked, &old))
        return 1;
    action.sa_handler = mark;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGUSR1, &action, &previous))
        return 1;
    pid_t parent = getpid(), child = fork();
    if (child < 0)
        return 1;
    if (!child)
    {
        _Exit(kill(parent, SIGUSR1) ? 1 : 0);
    }
    sigset_t waiting = old;
    sigdelset(&waiting, SIGUSR1);
    while (!seen)
        if (sigsuspend(&waiting) != -1 || errno != EINTR)
            return 1;
    int status;
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status) ||
        WEXITSTATUS(status))
        return 1;
    if (sigaction(SIGUSR1, &previous, NULL) ||
        sigprocmask(SIG_SETMASK, &old, NULL))
        return 1;
    puts("CONTROL-PASS:owned child signal,atomic unblock-and-wait,flag-only "
         "handler;mask/disposition restored");
    return 0;
}
