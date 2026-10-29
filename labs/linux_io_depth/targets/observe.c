/* Original local teaching probe. No network or external files. */
#define _GNU_SOURCE
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <sys/sendfile.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <linux/io_uring.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "IO_CHECK line=%d errno=%d\n", __LINE__, errno); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

int main(void)
{
    int pipes[2], ep, ev, timer, sfd, fd, dir, ring;
    char buffer[8], folder[] = "/tmp/cm-b06-XXXXXX";
    uint64_t value;
    sigset_t mask, old;
    struct signalfd_siginfo si;
    struct epoll_event reg = {.events = EPOLLIN | EPOLLET}, event;
    struct itimerspec when = {.it_value = {0, 1000000}};
    struct io_uring_params params = {0};
    CHECK(pipe2(pipes, O_NONBLOCK | O_CLOEXEC) == 0);
    ep = epoll_create1(EPOLL_CLOEXEC);
    CHECK(ep >= 0);
    reg.data.fd = pipes[0];
    CHECK(epoll_ctl(ep, EPOLL_CTL_ADD, pipes[0], &reg) == 0);
    CHECK(write(pipes[1], "abc", 3) == 3);
    CHECK(epoll_wait(ep, &event, 1, 1000) == 1);
    CHECK(read(pipes[0], buffer, 1) == 1);
    CHECK(epoll_wait(ep, &event, 1, 0) == 0);
    CHECK(read(pipes[0], buffer, 8) == 2);
    CHECK(read(pipes[0], buffer, 8) == -1 && errno == EAGAIN);
    puts("EPOLL partial-drain leaves bytes without a new edge; EAGAIN reached");
    ev = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    CHECK(ev >= 0);
    value = 3;
    CHECK(write(ev, &value, 8) == 8);
    CHECK(read(ev, &value, 8) == 8 && value == 3);
    close(ev);
    timer = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    CHECK(timer >= 0);
    CHECK(timerfd_settime(timer, 0, &when, NULL) == 0);
    CHECK(read(timer, &value, 8) == 8 && value >= 1);
    close(timer);
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    CHECK(sigprocmask(SIG_BLOCK, &mask, &old) == 0);
    sfd = signalfd(-1, &mask, SFD_CLOEXEC);
    CHECK(sfd >= 0);
    CHECK(kill(getpid(), SIGUSR1) == 0);
    CHECK(read(sfd, &si, sizeof si) == sizeof si && si.ssi_signo == SIGUSR1);
    close(sfd);
    CHECK(sigprocmask(SIG_SETMASK, &old, NULL) == 0);
    puts("EVENTFD TIMERFD SIGNALFD observed local events");
    CHECK(mkdtemp(folder) != NULL);
    dir = open(folder, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    CHECK(dir >= 0);
    fd = openat(dir, "staged", O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
    CHECK(fd >= 0);
    CHECK(write(fd, "xyz", 3) == 3);
    CHECK(fsync(fd) == 0);
    CHECK(renameat(dir, "staged", dir, "published") == 0);
    CHECK(fsync(dir) == 0);
    CHECK(lseek(fd, 0, SEEK_SET) == 0);
    CHECK(sendfile(pipes[1], fd, NULL, 3) == 3);
    CHECK(read(pipes[0], buffer, 8) == 3 && memcmp(buffer, "xyz", 3) == 0);
    close(fd);
    CHECK(unlinkat(dir, "published", 0) == 0);
    close(dir);
    CHECK(rmdir(folder) == 0);
    puts(
        "FSYNC file+directory and SENDFILE observed; no physical power-loss claim");
    ring = (int)syscall(SYS_io_uring_setup, 2, &params);
    if (ring < 0)
    {
        printf(
            "IO_URING unavailable errno=%d; operation/cancellation experiments pending\n",
            errno);
    }
    else
    {
        printf(
            "IO_URING setup features=%u; operation/cancellation experiments pending\n",
            params.features);
        close(ring);
    }
    close(ep);
    close(pipes[0]);
    close(pipes[1]);
    return 0;
}
