#include "common.h"
int hs_observe_clocks(struct hs_clocks *out) {
  if (!out)
    return -1;
  struct timespec w0, w1, c0, c1, res, pause = {0, 10000000};
  struct rlimit limit;
  if (clock_getres(CLOCK_MONOTONIC, &res) || getrlimit(RLIMIT_NOFILE, &limit) ||
      clock_gettime(CLOCK_MONOTONIC, &w0) ||
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &c0))
    return -1;
  while (nanosleep(&pause, &pause) && errno == EINTR) {
  }
  if (clock_gettime(CLOCK_MONOTONIC, &w1) ||
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &c1))
    return -1;
  *out = (struct hs_clocks){ns(&w1) - ns(&w0), ns(&c1) - ns(&c0), ns(&res),
                            (unsigned long)limit.rlim_cur};
  return 0;
}
int hs_limit_file(int fd, size_t limit) {
  if (fd < 0 || !limit || limit > 64)
    return -1;
  pid_t p = fork();
  if (p < 0)
    return -1;
  if (!p) {
    struct rlimit r = {(rlim_t)limit, (rlim_t)limit};
    if (setrlimit(RLIMIT_FSIZE, &r) || signal(SIGXFSZ, SIG_IGN) == SIG_ERR)
      _exit(1);
    char byte = 'x';
    size_t i;
    for (i = 0; i < limit + 1; i++) {
      ssize_t n = write(fd, &byte, 1);
      if (n < 0)
        break;
    }
    _exit(i == limit && errno == EFBIG ? 0 : 2);
  }
  int status;
  return reap(p, &status) || !WIFEXITED(status) || WEXITSTATUS(status) ? -1 : 0;
}
