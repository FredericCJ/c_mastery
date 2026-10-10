#include "common.h"
ssize_t hs_read_until(int fd, void *buffer, size_t cap, int cancel_fd,
                      long long deadline) {
  if (fd < 0 || cancel_fd < 0 || !buffer || !cap)
    return -1;
  for (;;) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
      return -1;
    long long current = ns(&t);
    if (deadline <= current)
      return -2;
    long long delta = deadline - current;
    long long ms = delta / 1000000 + (delta % 1000000 != 0);
    if (ms > INT_MAX)
      ms = INT_MAX;
    struct pollfd p[2] = {{fd, POLLIN, 0}, {cancel_fd, POLLIN, 0}};
    int r = poll(p, 2, (int)ms);
    if (r < 0 && errno == EINTR)
      continue;
    if (r < 0)
      return -1;
    if (!r)
      continue;
    if (p[1].revents & (POLLIN | POLLHUP))
      return -3;
    if (p[0].revents & POLLIN) {
      ssize_t n = read(fd, buffer, cap);
      if (n < 0 && (errno == EINTR || errno == EAGAIN))
        continue;
      return n;
    }
    if (p[0].revents & (POLLHUP | POLLERR | POLLNVAL))
      return p[0].revents & POLLHUP ? 0 : -1;
    if (p[1].revents & (POLLERR | POLLNVAL))
      return -1;
  }
}
