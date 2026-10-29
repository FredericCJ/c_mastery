#define _GNU_SOURCE
#include "hosted.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int temporary(void)
{
    char name[] = "/tmp/cmastery-hs-XXXXXX";
    int fd = mkstemp(name);
    assert(fd >= 0);
    assert(!unlink(name));
    return fd;
}

static void pipe_make(int p[2])
{
    assert(!pipe(p));
}

static void delay(void)
{
    struct timespec t = {0, 20000000};
    while (nanosleep(&t, &t) && errno == EINTR)
    {
    }
}

static long long now(void)
{
    struct timespec t;
    assert(!clock_gettime(CLOCK_MONOTONIC, &t));
    return (long long)t.tv_sec * 1000000000LL + t.tv_nsec;
}

static void descriptor(void)
{
    int f = temporary();
    assert(write(f, "abc", 3) == 3);
    assert(lseek(f, 0, SEEK_SET) == 0);
    int own = hs_dup_owned(f);
    assert(own >= 0 && own != f);
    assert(fcntl(own, F_GETFD) & FD_CLOEXEC);
    char c;
    assert(read(own, &c, 1) == 1 && c == 'a');
    assert(read(f, &c, 1) == 1 && c == 'b');
    assert(!close(own));
    assert(fcntl(f, F_GETFD) >= 0);
    assert(hs_dup_owned(-1) == -1);
    close(f);
}

static void copy_case(void)
{
    int p[2];
    pipe_make(p);
    pid_t child = fork();
    assert(child >= 0);
    if (!child)
    {
        close(p[0]);
        for (int i = 0; i < 200; i++)
            if (write(p[1], "xyz", 3) != 3)
                _exit(1);
        close(p[1]);
        _exit(0);
    }
    close(p[1]);
    int f = temporary();
    size_t n = 999;
    assert(!hs_copy(p[0], f, 1000, &n) && n == 600);
    close(p[0]);
    int status;
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
           WEXITSTATUS(status) == 0);
    assert(lseek(f, 0, SEEK_SET) == 0);
    char b[600];
    assert(read(f, b, 600) == 600);
    for (int i = 0; i < 600; i++)
        assert(b[i] == "xyz"[i % 3]);
    assert(lseek(f, 0, SEEK_SET) == 0);
    int dest = temporary();
    assert(!hs_copy(f, dest, 7, &n) && n == 7);
    struct stat st;
    assert(!fstat(dest, &st) && st.st_size == 7);
    assert(!hs_copy(-1, -1, 0, &n) && n == 0);
    assert(hs_copy(-1, dest, 1, &n) == -1 && n == 0);
    int full = open("/dev/full", O_WRONLY);
    assert(full >= 0);
    assert(hs_copy(f, full, 1, &n) == -1 && n == 0);
    close(full);
    close(f);
    close(dest);
}

static void deadline(void)
{
    int p[2], c[2];
    pipe_make(p);
    pipe_make(c);
    char b;
    assert(hs_read_until(p[0], &b, 1, c[0], LLONG_MIN) == -2);
    long long start = now();
    assert(hs_read_until(p[0], &b, 1, c[0], start + 20000000) == -2);
    assert(now() >= start + 15000000);
    assert(write(p[1], "x", 1) == 1);
    assert(hs_read_until(p[0], &b, 1, c[0], now() + 100000000) == 1 &&
           b == 'x');
    assert(write(p[1], "y", 1) == 1);
    assert(write(c[1], "!", 1) == 1);
    assert(hs_read_until(p[0], &b, 1, c[0], now() + 100000000) == -3);
    assert(read(p[0], &b, 1) == 1 && b == 'y');
    assert(hs_read_until(-1, &b, 1, c[0], now() + 1) == -1);
    for (int i = 0; i < 2; i++)
    {
        close(p[i]);
        close(c[i]);
    }
}

static void process(void)
{
    for (unsigned code = 0; code <= 255; code += 51)
    {
        int observed = -1;
        assert(!hs_spawn_exit(code, &observed) && observed == (int)code);
    }
    int observed = 123;
    assert(hs_spawn_exit(256, &observed) == -1 && observed == 123);
    int status;
    assert(waitpid(-1, &status, WNOHANG) == -1 && errno == ECHILD);
}

static void mapping(void)
{
    unsigned char *p = mmap(0, 4096, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(p != MAP_FAILED);
    memset(p, 0xa5, 4096);
    assert(!hs_shared_roundtrip(p, 4096, 64, "hello", 5));
    uint32_t off, len;
    memcpy(&off, p, 4);
    memcpy(&len, p + 4, 4);
    assert(off == 64 && len == 5 && !memcmp(p + off, "hello", len));
    assert(p[63] == 0xa5 && p[69] == 0xa5);
    assert(hs_shared_roundtrip(p, 4096, 4094, "bad", 3) == -1);
    assert(hs_shared_roundtrip(p, 4096, 4, "bad", 3) == -1);
    assert(!hs_shared_roundtrip(p, 4096, 4096, 0, 0));
    munmap(p, 4096);
}

static int frame_input(const unsigned char *b, size_t n)
{
    int p[2];
    pipe_make(p);
    for (size_t i = 0; i < n; i++)
        assert(write(p[1], b + i, 1) == 1);
    close(p[1]);
    return p[0];
}

static void frame(void)
{
    unsigned char input[] = {0, 3, 'a', 0, 'b'}, out[8] = {0};
    size_t n = 99;
    int f = frame_input(input, sizeof input);
    assert(!hs_read_frame(f, out, 8, &n) && n == 3 &&
           !memcmp(out, input + 2, 3));
    close(f);
    f = frame_input(input, 4);
    n = 77;
    assert(hs_read_frame(f, out, 8, &n) == -2 && n == 77);
    close(f);
    f = frame_input(input, 5);
    assert(hs_read_frame(f, out, 2, &n) == -3);
    close(f);
    unsigned char zero[] = {0, 0};
    f = frame_input(zero, 2);
    assert(!hs_read_frame(f, 0, 0, &n) && n == 0);
    close(f);
    f = frame_input(zero, 1);
    assert(hs_read_frame(f, out, 8, &n) == -2);
    close(f);
}

static void signal_case(void)
{
    sigset_t before, after;
    assert(!pthread_sigmask(SIG_SETMASK, 0, &before));
    assert(!hs_signal_roundtrip(SIGUSR1));
    assert(!hs_signal_roundtrip(SIGUSR2));
    assert(hs_signal_roundtrip(SIGKILL) == -1);
    assert(!pthread_sigmask(SIG_SETMASK, 0, &after));
    for (int sig = 1; sig < NSIG; sig++)
        assert(sigismember(&before, sig) == sigismember(&after, sig));
    sigset_t pending;
    assert(!sigpending(&pending));
    assert(!sigismember(&pending, SIGUSR1) && !sigismember(&pending, SIGUSR2));
}

static void ipc(void)
{
    unsigned char out[64] = {0}, input[64];
    for (unsigned i = 0; i < 64; i++)
        input[i] = (unsigned char)i;
    assert(hs_local_exchange(input, 64, out, 64) == 64);
    for (unsigned i = 0; i < 64; i++)
        assert(out[i] == 63 - i);
    assert(hs_local_exchange("abc", 3, out, 2) == -1);
    assert(hs_local_exchange(input, 65, out, 64) == -1);
    assert(hs_local_exchange(input, 0, out, 64) == 0);
}

static void reactor(void)
{
    int p[3][2], fds[3];
    for (int i = 0; i < 3; i++)
    {
        pipe_make(p[i]);
        fds[i] = p[i][0];
    }
    assert(write(p[2][1], "\3", 1) == 1);
    assert(write(p[0][1], "\1", 1) == 1);
    assert(write(p[1][1], "\2", 1) == 1);
    unsigned sum = 99;
    assert(!hs_reactor_sum(fds, 3, &sum) && sum == 6);
    for (int i = 0; i < 3; i++)
    {
        assert(fcntl(fds[i], F_GETFD) >= 0);
        close(p[i][1]);
    }
    sum = 99;
    assert(hs_reactor_sum(fds, 3, &sum) == -2 && sum == 99);
    for (int i = 0; i < 3; i++)
        close(fds[i]);
    assert(hs_reactor_sum(fds, 0, &sum) == -1);
}

static void pool(void)
{
    int values[256];
    long expected = 0;
    for (int i = 0; i < 256; i++)
    {
        values[i] = (i % 7) - 3;
        expected += values[i];
    }
    for (size_t workers = 1; workers <= 4; workers++)
    {
        long actual = 123;
        assert(!hs_pool_sum(values, 256, workers, &actual) &&
               actual == expected);
        assert(!hs_pool_sum(0, 0, workers, &actual) && actual == 0);
    }
    long sentinel = 77;
    assert(hs_pool_sum(values, 256, 0, &sentinel) == -1 && sentinel == 77);
    assert(hs_pool_sum(values, 257, 4, &sentinel) == -1);
    assert(hs_pool_sum(values, 256, 5, &sentinel) == -1);
}

struct consumer
{
    struct hs_queue *q;
    long sum;
    int count;
};

static void *consume(void *arg)
{
    struct consumer *c = arg;
    int v, r;
    while ((r = hs_queue_pop(c->q, &v)) == 1)
    {
        c->sum += v;
        c->count++;
    }
    assert(r == 0);
    return 0;
}

static void *blocked_push(void *arg)
{
    assert(hs_queue_push(arg, 99) == -1);
    return 0;
}

static void queue(void)
{
    struct hs_queue q;
    assert(!hs_queue_init(&q));
    struct consumer c = {&q, 0, 0};
    pthread_t t;
    assert(!pthread_create(&t, 0, consume, &c));
    for (int i = 0; i < 1000; i++)
        assert(!hs_queue_push(&q, i));
    assert(!hs_queue_close(&q));
    assert(!hs_queue_close(&q));
    assert(!pthread_join(t, 0));
    assert(c.count == 1000 && c.sum == 499500);
    assert(hs_queue_push(&q, 1) == -1);
    assert(!hs_queue_destroy(&q));
    assert(!hs_queue_init(&q));
    c = (struct consumer){&q, 0, 0};
    assert(!pthread_create(&t, 0, consume, &c));
    delay();
    pthread_mutex_lock(&q.mutex);
    pthread_cond_broadcast(&q.readable);
    pthread_mutex_unlock(&q.mutex);
    delay();
    assert(!hs_queue_close(&q));
    assert(!pthread_join(t, 0));
    assert(c.count == 0);
    assert(!hs_queue_destroy(&q));
    assert(!hs_queue_init(&q));
    for (int i = 0; i < 4; i++)
        assert(!hs_queue_push(&q, i));
    assert(!pthread_create(&t, 0, blocked_push, &q));
    delay();
    assert(!hs_queue_close(&q));
    assert(!pthread_join(t, 0));
    for (int i = 0; i < 4; i++)
    {
        int value;
        assert(hs_queue_pop(&q, &value) == 1 && value == i);
    }
    int value;
    assert(hs_queue_pop(&q, &value) == 0);
    assert(!hs_queue_destroy(&q));
}

static void clocks(void)
{
    struct hs_clocks c;
    struct rlimit before, after;
    assert(!getrlimit(RLIMIT_NOFILE, &before));
    assert(!hs_observe_clocks(&c));
    assert(c.wall_ns >= 7000000 && c.cpu_ns >= 0 && c.resolution_ns > 0 &&
           c.resolution_ns <= 1000000000);
    assert(c.fd_soft == (unsigned long)before.rlim_cur);
    assert(!getrlimit(RLIMIT_FSIZE, &before));
    int f = temporary();
    assert(!hs_limit_file(f, 8));
    struct stat st;
    assert(!fstat(f, &st) && st.st_size == 8);
    close(f);
    assert(!getrlimit(RLIMIT_FSIZE, &after));
    assert(before.rlim_cur == after.rlim_cur &&
           before.rlim_max == after.rlim_max);
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    if (!strcmp(argv[1], "descriptor"))
        descriptor();
    else if (!strcmp(argv[1], "copy"))
        copy_case();
    else if (!strcmp(argv[1], "deadline"))
        deadline();
    else if (!strcmp(argv[1], "process"))
        process();
    else if (!strcmp(argv[1], "mapping"))
        mapping();
    else if (!strcmp(argv[1], "frame"))
        frame();
    else if (!strcmp(argv[1], "signal"))
        signal_case();
    else if (!strcmp(argv[1], "ipc"))
        ipc();
    else if (!strcmp(argv[1], "reactor"))
        reactor();
    else if (!strcmp(argv[1], "pool"))
        pool();
    else if (!strcmp(argv[1], "queue"))
        queue();
    else if (!strcmp(argv[1], "clocks"))
        clocks();
    else
        return 2;
    puts(
        "actual hosted mechanism gate passed; complete project evidence remains");
    return 0;
}
