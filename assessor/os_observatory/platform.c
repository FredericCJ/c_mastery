#define _GNU_SOURCE
#include "oslab.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static int transfer(int fd, void *buffer, size_t n, int writing)
{
    size_t done = 0;
    while (done < n)
    {
        ssize_t got = writing ? write(fd, (char *)buffer + done, n - done)
                              : read(fd, (char *)buffer + done, n - done);
        if (got < 0 && errno == EINTR)
        {
            continue;
        }
        if (got <= 0)
        {
            return -1;
        }
        done += (size_t)got;
    }
    return 0;
}

int os_process_probe(void)
{
    int pipefd[2], status = 0, local = 7, received = 0;
    int *shared = mmap(NULL, sizeof *shared, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED)
    {
        return -1;
    }
    *shared = 0;
    if (pipe(pipefd))
    {
        munmap(shared, sizeof *shared);
        return -1;
    }
    pid_t child = fork();
    if (child == 0)
    {
        close(pipefd[0]);
        local = 9;
        *shared = 42;
        int rc = transfer(pipefd[1], &local, sizeof local, 1);
        close(pipefd[1]);
        _exit(rc ? 2 : 0);
    }
    close(pipefd[1]);
    int rc =
        child < 0 ? -1 : transfer(pipefd[0], &received, sizeof received, 0);
    close(pipefd[0]);
    pid_t waited = -1;
    if (child > 0)
    {
        do
        {
            waited = waitpid(child, &status, 0);
        } while (waited < 0 && errno == EINTR);
    }
    int ok = !rc && waited == child && WIFEXITED(status) &&
             WEXITSTATUS(status) == 0 && local == 7 && received == 9 &&
             *shared == 42;
    printf("process local=%d child_copy=%d shared=%d reaped=%d\n", local,
           received, *shared, waited == child);
    munmap(shared, sizeof *shared);
    return ok ? 0 : -1;
}

static void *counter_worker(void *arg)
{
    atomic_uint *value = arg;
    for (unsigned i = 0; i < 10000; ++i)
    {
        atomic_fetch_add_explicit(value, 1, memory_order_relaxed);
    }
    return arg;
}

int os_thread_probe(void)
{
    atomic_uint count;
    atomic_init(&count, 0);
    pthread_t threads[2];
    unsigned made = 0;
    for (; made < 2; ++made)
    {
        if (pthread_create(&threads[made], NULL, counter_worker, &count))
        {
            break;
        }
    }
    int ok = made == 2;
    for (unsigned i = 0; i < made; ++i)
    {
        void *result = NULL;
        if (pthread_join(threads[i], &result) || result != &count)
        {
            ok = 0;
        }
    }
    unsigned value = atomic_load_explicit(&count, memory_order_relaxed);
    printf("joined_threads=%u shared_counter=%u\n", made, value);
    return ok && value == 20000 ? 0 : -1;
}

int os_descriptor_probe(void)
{
    char path[] = "/tmp/cm-os-fd-XXXXXX", a = 0, b = 0;
    int fd = mkstemp(path);
    if (fd < 0)
    {
        return -1;
    }
    int ok = unlink(path) == 0;
    char content[] = "abcde";
    if (transfer(fd, content, 5, 1) || lseek(fd, 0, SEEK_SET) < 0)
    {
        ok = 0;
    }
    int alias = dup(fd);
    if (alias < 0 || transfer(fd, &a, 1, 0) || transfer(alias, &b, 1, 0))
    {
        ok = 0;
    }
    struct stat st;
    if (fstat(fd, &st) || st.st_nlink != 0)
    {
        ok = 0;
    }
    printf("dup_shared_offset=%d unlinked_open_object=%d\n",
           a == 'a' && b == 'b', ok);
    if (alias >= 0)
    {
        close(alias);
    }
    close(fd);
    return ok && a == 'a' && b == 'b' ? 0 : -1;
}

static volatile sig_atomic_t signal_fd = -1, signal_seen = 0;

static void handler(int sig)
{
    int saved = errno;
    char byte = '!';
    (void)sig;
    signal_seen = 1;
    if (signal_fd >= 0)
    {
        ssize_t result = write((int)signal_fd, &byte, 1);
        (void)result;
    }
    errno = saved;
}

int os_signal_probe(void)
{
    int fds[2];
    if (pipe2(fds, O_NONBLOCK | O_CLOEXEC))
    {
        return -1;
    }
    struct sigaction action = {0}, old;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    signal_fd = fds[1];
    signal_seen = 0;
    if (sigaction(SIGUSR1, &action, &old))
    {
        close(fds[0]);
        close(fds[1]);
        signal_fd = -1;
        return -1;
    }
    int rc = raise(SIGUSR1);
    struct pollfd p = {.fd = fds[0], .events = POLLIN};
    int ready = poll(&p, 1, 1000);
    char byte = 0;
    ssize_t n = read(fds[0], &byte, 1);
    int ok = !rc && ready == 1 && (p.revents & POLLIN) && signal_seen &&
             n == 1 && byte == '!';
    sigaction(SIGUSR1, &old, NULL);
    signal_fd = -1;
    close(fds[0]);
    close(fds[1]);
    printf("signal_to_readiness=%d\n", ok);
    return ok ? 0 : -1;
}

int os_memory_probe(void)
{
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0)
    {
        return -1;
    }
    size_t bytes = (size_t)page * 32;
    struct rusage before, after;
    volatile unsigned char *p = mmap(NULL, bytes, PROT_READ | PROT_WRITE,
                                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED)
    {
        return -1;
    }
    if (getrusage(RUSAGE_SELF, &before))
    {
        munmap((void *)p, bytes);
        return -1;
    }
    for (unsigned i = 0; i < 32; ++i)
    {
        p[(size_t)i * (size_t)page] = (unsigned char)(i + 1);
    }
    unsigned sum = 0;
    for (unsigned i = 0; i < 32; ++i)
    {
        sum += p[(size_t)i * (size_t)page];
    }
    int rc = getrusage(RUSAGE_SELF, &after);
    if (!rc)
    {
        printf(
            "touched_pages=32 checksum=%u minor_fault_delta=%ld major_fault_delta=%ld maxrss_kib=%ld\n",
            sum, after.ru_minflt - before.ru_minflt,
            after.ru_majflt - before.ru_majflt, after.ru_maxrss);
    }
    munmap((void *)p, bytes);
    return !rc && sum == 528 ? 0 : -1;
}

int os_io_probe(void)
{
    char path[] = "/tmp/cm-os-io-XXXXXX", written[8192], readback[8192];
    for (size_t i = 0; i < sizeof written; ++i)
    {
        written[i] = (char)(i % 127);
    }
    int fd = mkstemp(path);
    if (fd < 0)
    {
        return -1;
    }
    int ok = unlink(path) == 0 && !transfer(fd, written, sizeof written, 1) &&
             !fsync(fd) && lseek(fd, 0, SEEK_SET) == 0 &&
             !transfer(fd, readback, sizeof readback, 0) &&
             !memcmp(written, readback, sizeof written);
    close(fd);
    printf(
        "logical_write_bytes=8192 logical_read_bytes=8192 fsync_returned=%d physical_durability=unmeasured\n",
        ok);
    return ok ? 0 : -1;
}
