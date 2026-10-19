#include "ipc.h"
#include "signals.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
/* Original owned-process experiment. No general service accepts external peers.
 */
typedef struct {
  pthread_mutex_t lock;
  unsigned char request[4][8], response[4][4];
  unsigned request_epoch[4], response_epoch[4];
} Shared;
typedef struct {
  int rx, tx;
} Channel;
static int exact(int fd, void *buffer, size_t length, int sending,
                 struct timespec deadline) {
  unsigned char *p = buffer;
  while (length) {
    ssize_t n = sending ? write(fd, p, length) : read(fd, p, length);
    if (n > 0) {
      p += n;
      length -= (size_t)n;
      continue;
    }
    if (n == 0) {
      errno = EPIPE;
      return -1;
    }
    if (errno == EINTR)
      continue;
    if (errno != EAGAIN && errno != EWOULDBLOCK)
      return -1;
    struct timespec now, left;
    if (clock_gettime(CLOCK_MONOTONIC, &now) ||
        es_remaining(now, deadline, &left) <= 0) {
      errno = ETIMEDOUT;
      return -1;
    }
    struct pollfd wait = {fd, sending ? POLLOUT : POLLIN, 0};
    int r = ppoll(&wait, 1, &left, 0);
    if (r < 0 && errno != EINTR)
      return -1;
  }
  return 0;
}
static struct timespec limit(void) {
  struct timespec d;
  clock_gettime(CLOCK_MONOTONIC, &d);
  d.tv_sec += 5;
  return d;
}
static void close_channel(Channel c) {
  close(c.rx);
  if (c.tx != c.rx)
    close(c.tx);
}
static double seconds(struct timespec a, struct timespec b) {
  return (double)(b.tv_sec - a.tv_sec) + (double)(b.tv_nsec - a.tv_nsec) / 1e9;
}
static int server(Channel c, int mode, Shared *shared, unsigned rounds,
                  int gate) {
  alarm(12);
  char start;
  if (exact(gate, &start, 1, 0, limit()))
    return 2;
  close(gate);
  for (unsigned i = 0; i < rounds; ++i) {
    unsigned char input[8], output[4];
    unsigned slot = i % 4;
    if (mode == 2) {
      unsigned char ready;
      if (exact(c.rx, &ready, 1, 0, limit()) || ready != slot)
        return 3;
      if (pthread_mutex_lock(&shared->lock))
        return 4;
      if (shared->request_epoch[slot] != i + 1) {
        pthread_mutex_unlock(&shared->lock);
        return 5;
      }
      memcpy(input, shared->request[slot], 8);
      if (pthread_mutex_unlock(&shared->lock))
        return 6;
    } else if (exact(c.rx, input, 8, 0, limit()))
      return 7;
    ei_reply(input, output);
    if (mode == 2) {
      if (pthread_mutex_lock(&shared->lock))
        return 8;
      memcpy(shared->response[slot], output, 4);
      shared->response_epoch[slot] = i + 1;
      if (pthread_mutex_unlock(&shared->lock))
        return 9;
      unsigned char ready = (unsigned char)slot;
      if (exact(c.tx, &ready, 1, 1, limit()))
        return 10;
    } else if (exact(c.tx, output, 4, 1, limit()))
      return 11;
  }
  close_channel(c);
  return 0;
}
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c)) {                                                                \
      fprintf(stderr, "IPC-NATIVE-CHECK:%s:%d errno=%d\n", #c, __LINE__,       \
              errno);                                                          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(int argc, char **argv) {
  CHECK(argc == 3);
  int mode = atoi(argv[1]);
  unsigned rounds = (unsigned)strtoul(argv[2], 0, 10);
  CHECK(mode >= 0 && mode <= 2 && rounds >= 4 && rounds <= 4096 &&
        rounds % 4 == 0);
  alarm(15);
  Channel client, peer;
  int gate[2];
  CHECK(pipe2(gate, O_CLOEXEC | O_NONBLOCK) == 0);
  Shared *shared = 0;
  if (mode == 1) {
    int fd[2];
    CHECK(socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                     fd) == 0);
    client = (Channel){fd[0], fd[0]};
    peer = (Channel){fd[1], fd[1]};
  } else {
    int request[2], response[2];
    CHECK(pipe2(request, O_NONBLOCK | O_CLOEXEC) == 0 &&
          pipe2(response, O_NONBLOCK | O_CLOEXEC) == 0);
    client = (Channel){response[0], request[1]};
    peer = (Channel){request[0], response[1]};
  }
  if (mode == 2) {
    shared = mmap(0, sizeof *shared, PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    CHECK(shared != MAP_FAILED);
    pthread_mutexattr_t attributes;
    CHECK(pthread_mutexattr_init(&attributes) == 0);
    CHECK(pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED) ==
          0);
    CHECK(pthread_mutex_init(&shared->lock, &attributes) == 0);
    CHECK(pthread_mutexattr_destroy(&attributes) == 0);
  }
  pid_t child = fork();
  CHECK(child >= 0);
  if (!child) {
    close(gate[1]);
    close_channel(client);
    int result = server(peer, mode, shared, rounds, gate[0]);
    if (mode == 2)
      munmap(shared, sizeof *shared);
    _exit(result);
  }
  close(gate[0]);
  close_channel(peer);
  struct timespec start, end, cpu0, cpu1;
  CHECK(clock_gettime(CLOCK_MONOTONIC, &start) == 0);
  CHECK(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu0) == 0);
  unsigned completed = 0;
  size_t wire = 0;
  unsigned char bytes[8] = {0, 1, 2, 3, 128, 254, 255, 0};
  for (unsigned base = 0; base < rounds; base += 4) {
    EiQueue queue = {0};
    EiRequest requests[5];
    for (unsigned k = 0; k < 5; ++k)
      CHECK(ei_init(&requests[k], bytes, 1, base + k + 1) == 0);
    for (unsigned k = 0; k < 4; ++k)
      CHECK(ei_enqueue(&queue, &requests[k]) == 1);
    CHECK(ei_enqueue(&queue, &requests[4]) == 0 &&
          requests[4].owner == EI_CLIENT && queue.bytes == 32);
    for (unsigned k = 0; k < 4; ++k) {
      if (mode == 2) {
        CHECK(pthread_mutex_lock(&shared->lock) == 0);
        memcpy(shared->request[k], bytes, 8);
        shared->request_epoch[k] = base + k + 1;
        CHECK(pthread_mutex_unlock(&shared->lock) == 0);
        unsigned char token = (unsigned char)k;
        CHECK(exact(client.tx, &token, 1, 1, limit()) == 0);
        ++wire;
      } else {
        CHECK(exact(client.tx, bytes, 8, 1, limit()) == 0);
        wire += 8;
      }
    }
    if (!base) {
      char token = 'G';
      CHECK(write(gate[1], &token, 1) == 1);
      close(gate[1]);
    }
    for (unsigned k = 0; k < 4; ++k) {
      unsigned char answer[4];
      if (mode == 2) {
        unsigned char token;
        CHECK(exact(client.rx, &token, 1, 0, limit()) == 0 && token == k);
        CHECK(pthread_mutex_lock(&shared->lock) == 0);
        CHECK(shared->response_epoch[k] == base + k + 1);
        memcpy(answer, shared->response[k], 4);
        CHECK(pthread_mutex_unlock(&shared->lock) == 0);
        ++wire;
      } else {
        CHECK(exact(client.rx, answer, 4, 0, limit()) == 0);
        wire += 4;
      }
      CHECK(answer[0] == 0 && answer[1] == 0 && answer[2] == 2 &&
            answer[3] == 131);
      EiRequest *owned;
      CHECK(ei_dequeue(&queue, &owned) == 1 && owned == &requests[k]);
      CHECK(ei_transition(owned, EI_EFFECT) == 0 &&
            ei_transition(owned, EI_COMPLETE) == 0 && owned->result == 643);
      ++completed;
    }
    CHECK(queue.count == 0 && queue.bytes == 0);
  }
  CHECK(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu1) == 0);
  CHECK(clock_gettime(CLOCK_MONOTONIC, &end) == 0);
  close_channel(client);
  int status;
  struct rusage child_usage;
  CHECK(wait4(child, &status, 0, &child_usage) == child && WIFEXITED(status) &&
        WEXITSTATUS(status) == 0);
  if (mode == 2) {
    CHECK(pthread_mutex_destroy(&shared->lock) == 0);
    CHECK(munmap(shared, sizeof *shared) == 0);
  }
  printf("IPC-NATIVE:mode=%d;completed=%u;queue=4;retained_payload=32;fifth_"
         "rejected=%u;wire_bytes=%zu;shared_bytes=%zu;wall_seconds=%.9f;parent_"
         "cpu=%.9f;child_user=%ld.%06ld;child_system=%ld.%06ld;\n",
         mode, completed, rounds / 4, wire, mode == 2 ? sizeof(Shared) : 0,
         seconds(start, end), seconds(cpu0, cpu1),
         (long)child_usage.ru_utime.tv_sec, (long)child_usage.ru_utime.tv_usec,
         (long)child_usage.ru_stime.tv_sec, (long)child_usage.ru_stime.tv_usec);
  return 0;
}
