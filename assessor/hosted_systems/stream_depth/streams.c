#include "signals.h"
#include "stream.h"
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c)) {                                                                \
      fprintf(stderr, "STREAM-NATIVE-CHECK:%s:%d errno=%d\n", #c, __LINE__,    \
              errno);                                                          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static struct timespec limit(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  t.tv_sec += 3;
  return t;
}
static int pair(int fd[2]) {
  return socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fd);
}
static int transfer(int fd, unsigned char *b, size_t n, int sending) {
  struct timespec end = limit();
  size_t at = 0;
  while (at < n) {
    struct timespec now, left;
    if (clock_gettime(CLOCK_MONOTONIC, &now) ||
        es_remaining(now, end, &left) <= 0) {
      errno = ETIMEDOUT;
      return -1;
    }
    ssize_t done = sending ? send(fd, b + at, n - at, MSG_NOSIGNAL)
                           : recv(fd, b + at, n - at, 0);
    if (done > 0) {
      at += (size_t)done;
      continue;
    }
    if (!done) {
      errno = EPIPE;
      return -1;
    }
    if (errno == EINTR)
      continue;
    if (errno != EAGAIN && errno != EWOULDBLOCK)
      return -1;
    struct pollfd p = {fd, sending ? POLLOUT : POLLIN, 0};
    if (ppoll(&p, 1, &left, 0) < 0 && errno != EINTR)
      return -1;
  }
  return 0;
}
static int receive_frames(int fd, struct rf_parser *parser, struct rf_frame frames[2],
                          unsigned *count) {
  *count = 0;
  struct timespec end = limit();
  for (;;) {
    unsigned char bytes[2];
    ssize_t n = recv(fd, bytes, sizeof bytes, 0);
    if (n == 0)
      return rf_eof(parser);
    if (n < 0) {
      if (errno == EINTR)
        continue;
      if (errno != EAGAIN && errno != EWOULDBLOCK)
        return -1;
      if (es_wait_fd(fd, end) <= 0)
        return -1;
      continue;
    }
    size_t at = 0;
    while (at < (size_t)n) {
      size_t used;
      struct rf_frame frame;
      int r = rf_feed(parser, bytes + at, (size_t)n - at, &used, &frame);
      if (r < 0 || !used)
        return -1;
      at += used;
      if (r) {
        if (*count == 2)
          return -1;
        frames[(*count)++] = frame;
      }
    }
  }
}
static int half_close(void) {
  int fd[2];
  CHECK(pair(fd) == 0);
  unsigned char request[5] = {0, 3, 5, 0, 1};
  CHECK(transfer(fd[0], request, sizeof request, 1) == 0 &&
        shutdown(fd[0], SHUT_WR) == 0);
  struct rf_parser p;
  struct rf_frame frames[2];
  unsigned count;
  CHECK(rf_parser_init(&p, 1) == 0 &&
        receive_frames(fd[1], &p, frames, &count) == 0 && count == 2 &&
        frames[0].length == 0 && frames[1].length == 3 &&
        frames[1].bytes[0] == 5 && frames[1].bytes[1] == 0 &&
        frames[1].bytes[2] == 1);
  unsigned char reply[3] = {2, 'O', 'K'}, got[3];
  CHECK(transfer(fd[1], reply, 3, 1) == 0 && close(fd[1]) == 0);
  CHECK(transfer(fd[0], got, 3, 0) == 0 && !memcmp(got, reply, 3) &&
        recv(fd[0], got, 3, 0) == 0 && close(fd[0]) == 0);
  unsigned char incomplete[2][3] = {{0, 0, 0}, {0, 3, 'x'}};
  unsigned sizes[2] = {1, 3};
  for (unsigned i = 0; i < 2; ++i) {
    CHECK(pair(fd) == 0 && rf_parser_init(&p, 2) == 0);
    CHECK(transfer(fd[0], incomplete[i], sizes[i], 1) == 0 &&
          shutdown(fd[0], SHUT_WR) == 0);
    CHECK(receive_frames(fd[1], &p, frames, &count) == -1 && p.failed &&
          !count);
    CHECK(close(fd[0]) == 0 && close(fd[1]) == 0);
  }
  CHECK(pair(fd) == 0 && rf_parser_init(&p, 2) == 0);
  unsigned char fresh[3] = {0, 1, 42};
  CHECK(transfer(fd[0], fresh, 3, 1) == 0 && shutdown(fd[0], SHUT_WR) == 0 &&
        receive_frames(fd[1], &p, frames, &count) == 0 && count == 1 &&
        frames[0].length == 1 && frames[0].bytes[0] == 42);
  CHECK(close(fd[0]) == 0 && close(fd[1]) == 0);
  puts("STREAM-HALFCLOSE:response_after_client_SHUT_WR=1;mid_header_EOF_"
       "rejected=1;mid_body_EOF_rejected=1;reconnect_fresh_parser=1;");
  return 0;
}
static int replay(void) {
  struct rf_dedup cache;
  CHECK(rf_dedup_init(&cache, 9) == 0);
  for (unsigned attempt = 0; attempt < 2; ++attempt) {
    int fd[2];
    CHECK(pair(fd) == 0);
    unsigned char wire[3] = {2, 7, 3};
    CHECK(transfer(fd[0], wire, 3, 1) == 0 && shutdown(fd[0], SHUT_WR) == 0);
    struct rf_parser p;
    struct rf_frame frames[2];
    unsigned count;
    CHECK(rf_parser_init(&p, 1) == 0 &&
          receive_frames(fd[1], &p, frames, &count) == 0 && count == 1 &&
          frames[0].length == 2);
    uint32_t value;
    CHECK(rf_effect(&cache, 9, frames[0].bytes[0], frames[0].bytes[1],
                    &value) == (attempt ? 0 : 1) &&
          value == 3 && cache.total == 3);
    unsigned char reply[5] = {4, 0, 0, 0, (unsigned char)value}, out[5];
    if (attempt)
      CHECK(transfer(fd[1], reply, 5, 1) == 0);
    CHECK(close(fd[1]) == 0);
    if (attempt)
      CHECK(transfer(fd[0], out, 5, 0) == 0 && !memcmp(out, reply, 5));
    else
      CHECK(recv(fd[0], out, 5, 0) == 0);
    CHECK(close(fd[0]) == 0);
  }
  CHECK(cache.count == 1);
  puts("STREAM-REPLAY:connections=2;lost_reply_recovered=1;effect_count=1;"
       "stable_live_server_cache=1;cross_crash_exactly_once=0;");
  return 0;
}
static int pressure(void) {
  int fd[4][2];
  size_t prefill[4] = {0};
  unsigned short_writes = 0;
  unsigned char padding[1024] = {0};
  struct rf_outbox q;
  CHECK(rf_outbox_init(&q, 256) == 0);
  for (unsigned i = 0; i < 4; ++i) {
    CHECK(pair(fd[i]) == 0);
    int requested = 1024;
    CHECK(setsockopt(fd[i][0], SOL_SOCKET, SO_SNDBUF, &requested,
                     sizeof requested) == 0);
    unsigned slot;
    CHECK(rf_open(&q, &slot) == 1 && slot == i);
    for (;;) {
      ssize_t n = send(fd[i][0], padding, sizeof padding, MSG_NOSIGNAL);
      if (n > 0) {
        prefill[i] += (size_t)n;
        if ((size_t)n < sizeof padding)
          ++short_writes;
        CHECK(prefill[i] <= 65536);
        continue;
      }
      CHECK(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
      break;
    }
  }
  struct rf_message messages[36];
  for (unsigned i = 0; i < 36; ++i) {
    struct rf_frame f = {.length = 8};
    for (unsigned j = 0; j < 8; ++j)
      f.bytes[j] = (unsigned char)(i + j);
    CHECK(rf_message_init(&messages[i], i + 1, 2, &f) == 0);
    CHECK(rf_enqueue(&q, i % 4, &messages[i]) == (i < 32));
    CHECK(messages[i].owner == (i < 32 ? RF_OUTBOX : RF_CALLER));
  }
  CHECK(q.bytes == 256);
  for (unsigned i = 0; i < 4; ++i) {
    const unsigned char *bytes;
    size_t n;
    CHECK(rf_peek(&q, i, &bytes, &n) == 1 && n == 10);
    CHECK(send(fd[i][0], bytes, 1, MSG_NOSIGNAL) == -1 &&
          (errno == EAGAIN || errno == EWOULDBLOCK));
    CHECK(messages[i].offset == 0 && messages[i].owner == RF_OUTBOX);
    size_t left = prefill[i];
    while (left) {
      size_t chunk = left < sizeof padding ? left : sizeof padding;
      CHECK(transfer(fd[i][1], padding, chunk, 0) == 0);
      left -= chunk;
    }
  }
  struct rf_parser parsers[4];
  unsigned completed[4] = {0};
  for (unsigned i = 0; i < 4; ++i)
    CHECK(rf_parser_init(&parsers[i], 2) == 0);
  for (unsigned step = 0; step < 80; ++step)
    for (unsigned i = 0; i < 4; ++i) {
      const unsigned char *bytes;
      size_t n;
      CHECK(rf_peek(&q, i, &bytes, &n) == 1);
      CHECK(send(fd[i][0], bytes, 1, MSG_NOSIGNAL) == 1 &&
            rf_advance(&q, i, 1) == 0);
      unsigned char byte;
      CHECK(transfer(fd[i][1], &byte, 1, 0) == 0);
      struct rf_frame frame;
      int r = rf_push(&parsers[i], byte, &frame);
      CHECK(r >= 0);
      if (r) {
        CHECK(frame.length == 8);
        for (unsigned j = 0; j < 8; ++j)
          CHECK(frame.bytes[j] == i + 4 * completed[i] + j);
        ++completed[i];
      }
      CHECK(rf_outbox_invariant(&q));
    }
  CHECK(q.bytes == 0);
  for (unsigned i = 0; i < 32; ++i)
    CHECK(messages[i].owner == RF_DELIVERED);
  for (unsigned i = 0; i < 4; ++i)
    CHECK(completed[i] == 8 && rf_eof(&parsers[i]) == 0 &&
          rf_disconnect(&q, i) == 0 && close(fd[i][0]) == 0 &&
          close(fd[i][1]) == 0);
  /* A faster producer under zero drain: explicit global payload budget64. */
  CHECK(rf_outbox_init(&q, 64) == 0);
  for (unsigned i = 0; i < 4; ++i) {
    unsigned slot;
    CHECK(rf_open(&q, &slot) == 1);
  }
  unsigned admitted = 0, rejected = 0;
  size_t expected_bytes = 0;
  unsigned counts[4] = {0};
  for (unsigned attempt = 0; attempt < 256; ++attempt) {
    struct rf_frame f = {.length = attempt % 9};
    CHECK(admitted < 36 &&
          rf_message_init(&messages[admitted], attempt + 1, 2, &f) == 0);
    unsigned slot = attempt % 4;
    int expected = counts[slot] < 8 && expected_bytes + f.length <= 64;
    CHECK(rf_enqueue(&q, slot, &messages[admitted]) == expected);
    if (expected) {
      ++counts[slot];
      expected_bytes += f.length;
      ++admitted;
    } else {
      CHECK(messages[admitted].owner == RF_CALLER);
      ++rejected;
    }
    CHECK(q.bytes == expected_bytes && q.bytes <= 64);
  }
  for (unsigned i = 0; i < 4; ++i)
    CHECK(rf_disconnect(&q, i) == 0);
  CHECK(q.bytes == 0);
  for (unsigned i = 0; i < admitted; ++i)
    CHECK(messages[i].owner == RF_CANCELLED);
  printf("STREAM-PRESSURE:actual_connections=4;queued_frames=32;ninth_rejected="
         "4;would_block_owners_retained=4;one_byte_frame_progress=320;observed_"
         "short_prefill_syscalls=%u;global_budget=64;fast_admitted=%u;fast_"
         "rejected=%u;\n",
         short_writes, admitted, rejected);
  return 0;
}
int main(int argc, char **argv) {
  alarm(15);
  CHECK(argc == 2);
  if (!strcmp(argv[1], "halfclose"))
    return half_close();
  if (!strcmp(argv[1], "replay"))
    return replay();
  if (!strcmp(argv[1], "pressure"))
    return pressure();
  return 2;
}
