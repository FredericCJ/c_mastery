#ifndef HS_COMMON_H
#define HS_COMMON_H
#define _GNU_SOURCE
#include "hosted.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static inline int reap(pid_t p, int *status) {
  pid_t r;
  do {
    r = waitpid(p, status, 0);
  } while (r < 0 && errno == EINTR);
  return r == p ? 0 : -1;
}
static inline ssize_t retry_read(int fd, void *p, size_t n) {
  ssize_t r;
  do {
    r = read(fd, p, n);
  } while (r < 0 && errno == EINTR);
  return r;
}
static inline int write_all(int fd, const void *p, size_t n, size_t *done) {
  const unsigned char *b = p;
  while (*done < n) {
    ssize_t r = write(fd, b + *done, n - *done);
    if (r < 0 && errno == EINTR)
      continue;
    if (r <= 0)
      return -1;
    *done += (size_t)r;
  }
  return 0;
}
static inline long long ns(const struct timespec *t) {
  return (long long)t->tv_sec * 1000000000LL + t->tv_nsec;
}
static inline int read_exact(int fd, unsigned char *p, size_t n) {
  size_t done = 0;
  while (done < n) {
    ssize_t r = retry_read(fd, p + done, n - done);
    if (r < 0)
      return -1;
    if (!r)
      return -2;
    done += (size_t)r;
  }
  return 0;
}
#endif
