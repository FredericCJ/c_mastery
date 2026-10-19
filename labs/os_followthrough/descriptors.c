#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "OF_FD:%d:%s:errno%d\n", __LINE__, #x, errno);           \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int fd_count(void) {
  DIR *dir = opendir("/proc/self/fd");
  if (!dir)
    return -1;
  int n = 0;
  struct dirent *entry;
  while ((entry = readdir(dir)))
    if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
      n++;
  closedir(dir);
  return n;
}
static int offsets(void) {
  char path[] = "/tmp/of-fd-XXXXXX", bytes[12] = {0};
  int a = mkstemp(path);
  CHECK(a >= 0 && write(a, "abcdefghijkl", 12) == 12 &&
        lseek(a, 0, SEEK_SET) == 0);
  int b = dup(a), c = open(path, O_RDONLY | O_CLOEXEC);
  CHECK(b >= 0 && c >= 0 && unlink(path) == 0);
  CHECK(read(a, bytes, 3) == 3 && read(b, bytes, 2) == 2 &&
        read(c, bytes, 4) == 4);
  CHECK(lseek(a, 0, SEEK_CUR) == 5 && lseek(b, 0, SEEK_CUR) == 5 &&
        lseek(c, 0, SEEK_CUR) == 4);
  pid_t child = fork();
  CHECK(child >= 0);
  if (!child) {
    char value;
    _exit(read(b, &value, 1) == 1 && value == 'f' ? 0 : 3);
  }
  int status;
  CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
        !WEXITSTATUS(status) && lseek(a, 0, SEEK_CUR) == 6);
  int sockets[2];
  CHECK(socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sockets) == 0);
  char marker = 'x', control[CMSG_SPACE(sizeof(int))] = {0};
  struct iovec iov = {&marker, 1};
  struct msghdr message = {0};
  message.msg_iov = &iov;
  message.msg_iovlen = 1;
  message.msg_control = control;
  message.msg_controllen = sizeof control;
  struct cmsghdr *header = CMSG_FIRSTHDR(&message);
  header->cmsg_level = SOL_SOCKET;
  header->cmsg_type = SCM_RIGHTS;
  header->cmsg_len = CMSG_LEN(sizeof(int));
  memcpy(CMSG_DATA(header), &a, sizeof a);
  CHECK(sendmsg(sockets[0], &message, 0) == 1);
  memset(control, 0, sizeof control);
  CHECK(recvmsg(sockets[1], &message, MSG_CMSG_CLOEXEC) == 1);
  header = CMSG_FIRSTHDR(&message);
  CHECK(header && header->cmsg_level == SOL_SOCKET &&
        header->cmsg_type == SCM_RIGHTS);
  int received;
  memcpy(&received, CMSG_DATA(header), sizeof received);
  CHECK(received >= 0 && (fcntl(received, F_GETFD) & FD_CLOEXEC));
  CHECK(read(received, bytes, 1) == 1 && bytes[0] == 'g' &&
        lseek(a, 0, SEEK_CUR) == 7);
  CHECK(fcntl(b, F_SETFL, fcntl(b, F_GETFL) | O_NONBLOCK) == 0 &&
        (fcntl(a, F_GETFL) & O_NONBLOCK) && !(fcntl(c, F_GETFL) & O_NONBLOCK));
  CHECK(fcntl(b, F_SETFD, FD_CLOEXEC) == 0 &&
        !(fcntl(a, F_GETFD) & FD_CLOEXEC));
  int old = a;
  CHECK(close(a) == 0);
  a = open("/dev/null", O_RDONLY | O_CLOEXEC);
  CHECK(a == old);
  CHECK(close(a) == 0 && close(b) == 0 && close(c) == 0 &&
        close(received) == 0 && close(sockets[0]) == 0 &&
        close(sockets[1]) == 0);
  puts("OF:FD:offsets5,5,4:fork6:SCM_RIGHTS7:description-status:descriptor-"
       "CLOEXEC:numeric-reuse");
  return 0;
}
static int eof(void) {
  int data[2], release[2], ready[2];
  CHECK(pipe2(data, O_NONBLOCK | O_CLOEXEC) == 0 &&
        pipe2(release, O_CLOEXEC) == 0 && pipe2(ready, O_CLOEXEC) == 0);
  pid_t child = fork();
  CHECK(child >= 0);
  if (!child) {
    close(data[0]);
    close(release[1]);
    close(ready[0]);
    char value = '!';
    if (write(ready[1], &value, 1) != 1)
      _exit(2);
    if (read(release[0], &value, 1) != 1)
      _exit(3);
    close(data[1]);
    close(release[0]);
    close(ready[1]);
    _exit(0);
  }
  CHECK(close(data[1]) == 0 && close(release[0]) == 0 && close(ready[1]) == 0);
  char value;
  CHECK(read(ready[0], &value, 1) == 1);
  CHECK(read(data[0], &value, 1) == -1 && errno == EAGAIN);
  CHECK(write(release[1], "x", 1) == 1);
  int status;
  CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
        !WEXITSTATUS(status));
  CHECK(read(data[0], &value, 1) == 0);
  CHECK(close(data[0]) == 0 && close(release[1]) == 0 && close(ready[0]) == 0);
  puts("OF:EOF:inherited-writer-blocks:handshake-last-close-then-zero");
  return 0;
}
static int acquisition(unsigned fail) {
  int before = fd_count(), fds[7];
  for (unsigned i = 0; i < 7; i++)
    fds[i] = -1;
  unsigned stage = 0;
  pid_t child = -1;
  char path[] = "/tmp/of-acquire-XXXXXX";
  fds[0] = mkstemp(path);
  CHECK(fds[0] >= 0 && unlink(path) == 0);
  if (++stage == fail)
    goto cleanup;
  CHECK(pipe2(&fds[1], O_CLOEXEC) == 0);
  if (++stage == fail)
    goto cleanup;
  fds[3] = dup(fds[0]);
  CHECK(fds[3] >= 0);
  if (++stage == fail)
    goto cleanup;
  CHECK(pipe2(&fds[4], O_CLOEXEC) == 0);
  if (++stage == fail)
    goto cleanup;
  child = fork();
  CHECK(child >= 0);
  if (!child) {
    close(fds[2]);
    close(fds[0]);
    close(fds[3]);
    close(fds[4]);
    close(fds[5]);
    char x;
    ssize_t n = read(fds[1], &x, 1);
    close(fds[1]);
    _exit(n == 0 ? 0 : 2);
  }
  ++stage;
cleanup:
  for (unsigned i = 0; i < 7; i++)
    if (fds[i] >= 0)
      CHECK(close(fds[i]) == 0);
  if (child > 0) {
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          !WEXITSTATUS(status));
  }
  CHECK(fd_count() == before);
  printf(
      "OF:FD-failure:stage%u:requested%u:descriptors-restored:child-reaped%d\n",
      stage, fail, child > 0);
  return 0;
}
static int exec_private(const char *self, unsigned cloexec) {
  int p[2];
  CHECK(pipe2(p, cloexec ? O_CLOEXEC : 0) == 0);
  char value[24];
  snprintf(value, sizeof value, "%d", p[0]);
  pid_t child = fork();
  CHECK(child >= 0);
  if (!child) {
    execl(self, self, "check-fd", value, (char *)NULL);
    _exit(9);
  }
  int status;
  CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
        WEXITSTATUS(status) == (cloexec ? 0 : 1));
  CHECK(close(p[0]) == 0 && close(p[1]) == 0);
  printf("OF:exec:CLOEXEC%u:private-endpoint-absent%d\n", cloexec,
         cloexec != 0);
  return 0;
}
int main(int argc, char **argv) {
  if (argc == 3 && !strcmp(argv[1], "check-fd")) {
    int fd = atoi(argv[2]);
    return fcntl(fd, F_GETFD) == -1 && errno == EBADF ? 0 : 1;
  }
  if (argc != 1)
    return 2;
  int before = fd_count();
  if (offsets() || eof() || exec_private(argv[0], 0) ||
      exec_private(argv[0], 1))
    return 1;
  for (unsigned fail = 1; fail <= 6; fail++)
    if (acquisition(fail))
      return 1;
  CHECK(fd_count() == before);
  puts("OF_PASS:descriptors");
  return 0;
}
