#define _POSIX_C_SOURCE 200809L
#include "linux.h"
#include "stream.h"
#include "test.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
static ptrdiff_t write_one(void *p, const unsigned char *b, size_t n) {
  int fd = *(int *)p;
  (void)n;
  ssize_t x = write(fd, b, 1);
  return x < 0 ? -1 : x;
}
static void assemble(int fd, const unsigned char expected[3]) {
  struct ar_reader reader = {{0}, 0, 1};
  struct ar_record r = {99};
  for (unsigned i = 0; i < 3; ++i) {
    unsigned char b = 0;
    struct ar_result got = ar_linux_read(fd, &b, 1);
    CHECK(got.state == AR_OK && got.count == 1 && b == expected[i]);
    CHECK(ar_feed(&reader, &b, 1, &r) == (i == 2 ? AR_OK : AR_PENDING));
  }
  CHECK(r.value == 513);
}
int main(void) {
  char path[] = "/tmp/cmastery-architecture-XXXXXX";
  int fd = mkstemp(path);
  CHECK(fd >= 0 && unlink(path) == 0);
  unsigned char wire[3];
  CHECK(ar_encode(1, (struct ar_record){513}, wire, 3) == AR_OK);
  CHECK(write(fd, wire, 3) == 3 && lseek(fd, 0, SEEK_SET) == 0);
  assemble(fd, wire);
  unsigned char b;
  CHECK(ar_linux_read(fd, &b, 1).state == AR_EMPTY);
  CHECK(close(fd) == 0);
  int pipefd[2];
  CHECK(pipe(pipefd) == 0 && fcntl(pipefd[0], F_SETFL, O_NONBLOCK) == 0);
  CHECK(ar_linux_read(pipefd[0], &b, 1).state == AR_PENDING);
  CHECK(write(pipefd[1], wire, 1) == 1 && write(pipefd[1], wire + 1, 2) == 2);
  assemble(pipefd[0], wire);
  CHECK(ar_linux_read(pipefd[0], &b, 1).state == AR_PENDING);
  struct sf_stream s;
  struct sf_handle handle;
  sf_init(&s);
  unsigned char payload[] = {1, 2, 3};
  CHECK(sf_start(&s, payload, 3, &handle) == SF_OK);
  payload[0] = 77;
  size_t accepted = 0;
  CHECK(sf_pump(handle, write_one, &pipefd[1], 1, &accepted) == SF_PENDING &&
        accepted == 1);
  CHECK(sf_pump(handle, write_one, &pipefd[1], 8, &accepted) == SF_DONE &&
        accepted == 5);
  unsigned char frame[5];
  CHECK(read(pipefd[0], frame, 5) == 5);
  CHECK(!memcmp(frame, (unsigned char[]){3, 1, 2, 3, 6}, 5));
  CHECK(close(pipefd[1]) == 0 &&
        ar_linux_read(pipefd[0], &b, 1).state == AR_EMPTY &&
        close(pipefd[0]) == 0);
  char journal[] = "/tmp/cmastery-outbox-XXXXXX";
  fd = mkstemp(journal);
  CHECK(fd >= 0 && unlink(journal) == 0);
  pid_t child = fork();
  CHECK(child >= 0);
  if (!child) {
    int result = ar_outbox(fd, 7, 123, 1);
    _exit(result == AR_PENDING ? 0 : 1);
  }
  int status;
  CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
        WEXITSTATUS(status) == 0);
  CHECK(ar_outbox(fd, 7, 123, 0) == AR_OK);
  CHECK(ar_outbox(fd, 7, 124, 0) == AR_STATE &&
        ar_outbox(fd, 8, 123, 0) == AR_STATE);
  unsigned char snapshot[12], after[12];
  CHECK(pread(fd, snapshot, 12, 0) == 12);
  CHECK(ar_outbox(fd, 7, 123, 1) == AR_OK && pread(fd, after, 12, 0) == 12 &&
        !memcmp(snapshot, after, 12));
  CHECK(ftruncate(fd, 0) == 0);
  CHECK(ar_outbox(fd, 7, 123, 0) == AR_OK);
  CHECK(ftruncate(fd, 5) == 0 && ar_outbox(fd, 7, 123, 0) == AR_FAILED);
  CHECK(close(fd) == 0);
  puts("PASS Linux actual file/nonblocking pipe,SFStream copy,child lost-ack "
       "recovery,torn record rejection");
}
