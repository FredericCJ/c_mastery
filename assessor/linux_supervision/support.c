#define _POSIX_C_SOURCE 200809L
#include "support.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

_Noreturn void fail_sys(const char *operation)
{
    perror(operation);
    _exit(1);
}

_Noreturn void fail_protocol(const char *operation)
{
    fprintf(stderr, "supervision: %s\n", operation);
    _exit(1);
}

int64_t now_ms(void)
{
    struct timespec sample;
    if (clock_gettime(CLOCK_MONOTONIC, &sample)) fail_sys("clock_gettime");
    return (int64_t)sample.tv_sec * 1000 + sample.tv_nsec / 1000000;
}

void await_fd(int fd, short events, int64_t deadline)
{
    struct pollfd description = {.fd = fd, .events = events};
    for (;;) {
        int64_t remaining = deadline - now_ms();
        if (remaining <= 0) fail_protocol("descriptor deadline expired");
        int result = poll(&description, 1, remaining > INT_MAX ? INT_MAX : (int)remaining);
        if (result < 0 && errno == EINTR) continue;
        if (result < 0) fail_sys("poll");
        if (!result) fail_protocol("descriptor deadline expired");
        if (description.revents & POLLNVAL) fail_protocol("poll on closed descriptor");
        return;
    }
}

void read_full(int fd, void *data, size_t size, int64_t deadline)
{
    unsigned char *cursor = data;
    while (size) {
        await_fd(fd, POLLIN, deadline);
        ssize_t n = read(fd, cursor, size);
        if (n < 0 && (errno == EINTR || errno == EAGAIN)) continue;
        if (n < 0) fail_sys("read");
        if (!n) fail_protocol("unexpected EOF");
        size -= (size_t)n;
        cursor += n;
    }
}

void write_full(int fd, const void *data, size_t size, int64_t deadline)
{
    const unsigned char *cursor = data;
    while (size) {
        await_fd(fd, POLLOUT, deadline);
        ssize_t n = write(fd, cursor, size);
        if (n < 0 && (errno == EINTR || errno == EAGAIN)) continue;
        if (n < 0) fail_sys("write");
        if (!n) fail_protocol("write made no progress");
        size -= (size_t)n;
        cursor += n;
    }
}

int collect(pid_t child, int64_t deadline)
{
    int status;
    for (;;) {
        pid_t result = waitpid(child, &status, WNOHANG);
        if (result == child) return status;
        if (result < 0 && errno != EINTR) fail_sys("waitpid");
        if (now_ms() >= deadline) fail_protocol("child did not terminate before deadline");
        struct timespec delay = {.tv_nsec = 1000000};
        nanosleep(&delay, NULL);
    }
}

void read_field(const char *path, char *out, size_t capacity)
{
    FILE *file = fopen(path, "r");
    if (!file) fail_sys(path);
    if (!fgets(out, (int)capacity, file)) fail_protocol("missing field");
    if (!strchr(out, '\n') && !feof(file)) fail_protocol("field too long");
    out[strcspn(out, "\n")] = '\0';
    if (fclose(file)) fail_sys("fclose");
}

void leaf_membership(char *out, size_t capacity)
{
    char membership[256];
    read_field("/proc/self/cgroup", membership, sizeof membership);
    if (strncmp(membership, "0::/lab/", 8) || strlen(membership + 3) >= capacity ||
        strstr(membership, "..")) fail_protocol("unexpected guest cgroup membership");
    strcpy(out, membership + 3);
}

static void leaf_path(const char *leaf, const char *name, char *path, size_t capacity)
{
    int length = snprintf(path, capacity, "/sys/fs/cgroup%s/%s", leaf, name);
    if (length < 0 || (size_t)length >= capacity) fail_protocol("cgroup path too long");
}

void leaf_field(const char *leaf, const char *name, char *out, size_t capacity)
{
    char path[512];
    leaf_path(leaf, name, path, sizeof path);
    read_field(path, out, capacity);
}

unsigned long long leaf_counter(const char *leaf, const char *file, const char *key)
{
    char path[512], line[256];
    leaf_path(leaf, file, path, sizeof path);
    FILE *input = fopen(path, "r");
    if (!input) fail_sys(path);
    while (fgets(line, sizeof line, input)) {
        char name[64];
        unsigned long long value;
        if (key ? (sscanf(line, "%63s %llu", name, &value) == 2 && !strcmp(key, name))
                : (sscanf(line, "%llu", &value) == 1)) {
            if (fclose(input)) fail_sys("fclose");
            return value;
        }
    }
    fail_protocol("cgroup counter missing");
}

unsigned long long namespace_inode(const char *name)
{
    char path[128];
    struct stat info;
    snprintf(path, sizeof path, "/proc/self/ns/%s", name);
    if (stat(path, &info)) fail_sys(path);
    return (unsigned long long)info.st_ino;
}

int count_fds(void)
{
    int count = 0;
    DIR *dir = opendir("/proc/self/fd");
    if (!dir) fail_sys("opendir /proc/self/fd");
    int own = dirfd(dir);
    struct dirent *ent;
    errno = 0;
    while ((ent = readdir(dir))) {
        char *end;
        long fd = strtol(ent->d_name, &end, 10);
        if (*end == '\0' && fd != own) ++count;
    }
    if (errno) fail_sys("readdir /proc/self/fd");
    if (closedir(dir)) fail_sys("closedir /proc/self/fd");
    return count;
}
