#define _GNU_SOURCE
#include "worker.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Original worker: reports only synthetic arguments and handle rights to fd0.
 */
int main(int argc, char **argv) {
  int admin = 0, options = 1;
  char bytes[4];
  int bad = 0;
  DIR *directory = opendir("/proc/self/fd");
  if (!directory)
    return 42;
  struct dirent *entry;
  unsigned seen = 0;
  while ((entry = readdir(directory))) {
    if (entry->d_name[0] == '.')
      continue;
    char *end;
    long fd = strtol(entry->d_name, &end, 10);
    if (*end || fd == dirfd(directory))
      continue;
    dprintf(0, "FD %ld\n", fd);
    if (fd == 0)
      seen |= 1;
    else if (fd == 4)
      seen |= 2;
    else
      bad = 1;
  }
  if (closedir(directory) || seen != 3)
    bad = 1;
  if (!getenv("SD_ONLY") || strcmp(getenv("SD_ONLY"), "yes") || getenv("PATH"))
    bad = 1;
  for (int i = 1; i < argc; ++i) {
    dprintf(0, "ARG %zu:%s\n", strlen(argv[i]), argv[i]);
    if (options && !strcmp(argv[i], "--")) {
      options = 0;
      continue;
    }
    if (options && !strcmp(argv[i], "--admin"))
      admin = 1;
  }
  if (sd_request(1, SD_READ_LOG, 4, bytes) || memcmp(bytes, "SAFE", 4))
    bad = 1;
  if (sd_request(1, SD_WRITE_CONFIG, 4, bytes) != EACCES ||
      sd_request(0, SD_READ_LOG, -1, bytes) != EACCES)
    bad = 1;
  errno = 0;
  ssize_t n = write(4, "X", 1);
  int e = errno;
  if (n != -1 || e != EBADF)
    bad = 1;
  const char *path = getenv("SD_REOPEN");
  if (path) {
    int fd = open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0 || pwrite(fd, "OPEN", 4, 0) != 4)
      bad = 1;
    if (fd >= 0 && close(fd))
      bad = 1;
    dprintf(0, "REOPEN ambient_write=observed\n");
  }
  dprintf(0, "WORKER admin=%d read=SAFE write=%zd errno=%d inventory=%s\n",
          admin, n, e, bad ? "FAIL" : "PASS");
  return bad ? 41 : 0;
}
