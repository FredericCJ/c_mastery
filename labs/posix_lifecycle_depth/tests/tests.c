#define _GNU_SOURCE
#include "posix_lifecycle.h"
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"PL_CHECK:%d:%s\n",__LINE__,#x); return 1; } } while (0)
static int descriptors(void) {
  DIR *directory = opendir("/proc/self/fd");
  if (!directory) return -1;
  int count = 0;
  struct dirent *entry;
  while ((entry = readdir(directory)))
    if (strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")) ++count;
  closedir(directory);
  return count;
}
int main(int argc, char **argv) {
  if (argc != 3) return 2;
  if (!strcmp(argv[1],"launch")) {
    int initial = descriptors();
    CHECK(initial >= 0);
    char *args[] = {argv[2], "zero", NULL};
    struct pl_launch result = {PL_SIGNALED, 999};
    CHECK(pl_launch(argv[2],args,0,&result) == 0);
    CHECK(result.kind == PL_EXITED && result.value == 0);
    args[1] = "127";
    CHECK(pl_launch(argv[2],args,0,&result) == 0);
    CHECK(result.kind == PL_EXITED && result.value == 127);
    char absent[4096];
    int length = snprintf(absent,sizeof absent,"%s.missing",argv[2]);
    CHECK(length > 0 && (size_t)length < sizeof absent);
    struct stat metadata;
    errno = 0;
    CHECK(lstat(absent,&metadata) == -1 && errno == ENOENT);
    CHECK(pl_launch(absent,args,0,&result) == 0);
    CHECK(result.kind == PL_EXEC_ERROR && result.value == ENOENT);
    args[1] = "signal";
    CHECK(pl_launch(argv[2],args,0,&result) == 0);
    CHECK(result.kind == PL_SIGNALED && result.value == SIGTERM);
    for (unsigned fail = 1; fail <= 2; ++fail) {
      result = (struct pl_launch){PL_SIGNALED, 999};
      CHECK(pl_launch(argv[2],args,fail,&result) == (fail == 1 ? EMFILE : EAGAIN));
      CHECK(result.kind == PL_SIGNALED && result.value == 999);
      CHECK(descriptors() == initial);
    }
    CHECK(pl_launch(NULL,args,0,&result) == EINVAL);
    CHECK(descriptors() == initial);
    errno = 0;
    CHECK(waitpid(-1,NULL,WNOHANG) == -1 && errno == ECHILD);
  } else if (!strcmp(argv[1],"pool")) {
    for (unsigned n = 0; n <= 4; ++n) {
      for (unsigned fail = 0; fail <= n; ++fail) {
        struct pl_pool result = {99,99,99,99,99};
        unsigned made = fail ? fail-1 : n, expected = 0;
        for (unsigned i = 1; i <= made; ++i) expected += i*i;
        CHECK(pl_pool(n,fail,&result) == 0);
        CHECK(result.created == made && result.joined == made && result.completed == made);
        CHECK(result.sum == expected && result.create_error == (fail ? EAGAIN : 0));
      }
    }
    struct pl_pool sentinel = {99,99,99,99,99}, result = sentinel;
    CHECK(pl_pool(5,0,&result) == EINVAL);
    CHECK(result.created == 99 && result.sum == 99 && result.create_error == 99);
    CHECK(pl_pool(1,2,&result) == EINVAL);
    CHECK(pl_pool(1,0,NULL) == EINVAL);
  } else return 2;
  printf("PASS posix_lifecycle_depth.%s\n",argv[1]);
  return 0;
}
