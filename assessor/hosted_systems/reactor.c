#include "common.h"
int hs_reactor_sum(const int *fds, size_t n, unsigned *sum) {
  if (!fds || !sum || !n || n > 8)
    return -1;
  struct pollfd p[8];
  for (size_t i = 0; i < n; i++) {
    if (fds[i] < 0)
      return -1;
    p[i] = (struct pollfd){fds[i], POLLIN, 0};
  }
  size_t left = n;
  unsigned total = 0;
  while (left) {
    int r = poll(p, (nfds_t)n, 1000);
    if (r < 0 && errno == EINTR)
      continue;
    if (r <= 0)
      return -1;
    for (size_t i = 0; i < n; i++) {
      if (p[i].fd < 0)
        continue;
      if (p[i].revents & POLLIN) {
        unsigned char x;
        if (retry_read(p[i].fd, &x, 1) != 1)
          return -2;
        total += x;
        p[i].fd = -1;
        left--;
      } else if (p[i].revents & (POLLHUP | POLLERR | POLLNVAL))
        return -2;
    }
  }
  *sum = total;
  return 0;
}
