#define _POSIX_C_SOURCE 200809L
#include "com_http.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
struct edu_output {
  unsigned char bytes[1600];
  size_t length, offset;
  uint64_t started;
};
struct edu_connection {
  int fd, closing, draining, eof;
  struct http_parser parser;
  unsigned char input[2048];
  size_t have, used;
  struct edu_output output[2];
  unsigned head, count, requests;
  uint64_t started, progress, drain_started;
};
static struct edu_connection connections[4];
static volatile sig_atomic_t stop;
static void interrupted(int sig) {
  (void)sig;
  stop = 1;
}
static uint64_t now_ms(void) {
  struct timespec t;
  if (clock_gettime(CLOCK_MONOTONIC, &t))
    abort();
  return (uint64_t)t.tv_sec * 1000 + (unsigned)t.tv_nsec / 1000000;
}
static int nonblock(int fd) {
  int a = fcntl(fd, F_GETFL), b = fcntl(fd, F_GETFD);
  return a < 0 || b < 0 || fcntl(fd, F_SETFL, a | O_NONBLOCK) < 0 ||
                 fcntl(fd, F_SETFD, b | FD_CLOEXEC) < 0
             ? -1
             : 0;
}
static void release(struct edu_connection *c) {
  if (c->fd >= 0)
    close(c->fd);
  memset(c, 0, sizeof *c);
  c->fd = -1;
}
static int send_part(void *ctx, const void *p, size_t n) {
  struct edu_connection *c = ctx;
  ssize_t r = send(c->fd, p, n, MSG_NOSIGNAL);
  if (r >= 0)
    return (int)r;
  if (errno == EINTR)
    return -1;
  if (errno == EAGAIN || errno == EWOULDBLOCK)
    return -2;
  return -3;
}
static int enqueue(struct edu_connection *c, int interim, uint64_t now) {
  if (c->count == 2)
    return -1;
  struct edu_output *o = &c->output[(c->head + c->count) % 2];
  o->offset = 0;
  o->started = now;
  if (interim) {
    static const char msg[] = "HTTP/1.1 100 Continue\r\n\r\n";
    memcpy(o->bytes, msg, sizeof msg - 1);
    o->length = sizeof msg - 1;
  } else {
    c->requests++;
    if (c->requests >= 4)
      c->parser.request.close = 1;
    c->closing = c->parser.request.close || c->eof;
    /* Clock injection is deliberately deterministic for the bounded lab. */
    if (http_response(&c->parser.request, "Sat, 10 Oct 2026 00:00:00 GMT",
                      c->closing, o->bytes, sizeof o->bytes, &o->length))
      return -1;
  }
  c->count++;
  return 0;
}
static int parse(struct edu_connection *c, uint64_t now, const char *authority) {
  while (c->used < c->have && c->count < 2 && !c->closing) {
    size_t n = 0;
    int r = http_feed(&c->parser, c->input + c->used, c->have - c->used, &n);
    c->used += n;
    if (r == HTTP_CONTINUE) {
      if (enqueue(c, 1, now))
        return -1;
    } else if (r == HTTP_DONE || r == HTTP_ERROR) {
      if (enqueue(c, 0, now))
        return -1;
      if (!c->closing) {
        http_init(&c->parser, authority);
        c->started = now;
      }
    } else if (!n)
      break;
  }
  if (c->used == c->have)
    c->used = c->have = 0;
  return 0;
}
int main(int argc, char **argv) {
  unsigned long port = 8080;
  if (argc == 2) {
    char *end;
    port = strtoul(argv[1], &end, 10);
    if (*end || port > 65535)
      return 2;
  } else if (argc != 1)
    return 2;
  int listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0 || nonblock(listener))
    return 2;
  int one = 1;
  setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons((uint16_t)port);
  if (bind(listener, (struct sockaddr *)&addr, sizeof addr) ||
      listen(listener, 4))
    return 2;
  socklen_t alen = sizeof addr;
  if (getsockname(listener, (struct sockaddr *)&addr, &alen))
    return 2;
  char authority[128];
  snprintf(authority, sizeof authority, "127.0.0.1:%u", ntohs(addr.sin_port));
  for (unsigned i = 0; i < 4; i++)
    connections[i].fd = -1;
  signal(SIGINT, interrupted);
  signal(SIGTERM, interrupted);
  printf("READY %s\n", authority);
  fflush(stdout);
  while (!stop) {
    struct pollfd f[5] = {{listener, POLLIN, 0}};
    for (unsigned i = 0; i < 4; i++) {
      struct edu_connection *c = &connections[i];
      short events = 0;
      if (c->fd >= 0) {
        if (c->draining || (!c->closing && c->count < 2 && c->have == 0))
          events |= POLLIN;
        if (c->count)
          events |= POLLOUT;
      }
      f[i + 1] = (struct pollfd){c->fd, events, 0};
    }
    int ready = poll(f, 5, 20);
    if (ready < 0) {
      if (errno == EINTR)
        continue;
      break;
    }
    uint64_t now = now_ms();
    if (f[0].revents & POLLIN) {
      for (unsigned attempts = 0; attempts < 4; attempts++) {
        int fd = accept(listener, 0, 0);
        if (fd < 0) {
          if (errno == EINTR)
            continue;
          break;
        }
        unsigned i;
        for (i = 0; i < 4; i++)
          if (connections[i].fd < 0)
            break;
        if (i == 4 || nonblock(fd)) {
          close(fd);
          continue;
        }
        struct edu_connection *c = &connections[i];
        memset(c, 0, sizeof *c);
        c->fd = fd;
        c->started = c->progress = now;
        http_init(&c->parser, authority);
      }
    }
    for (unsigned i = 0; i < 4; i++) {
      struct edu_connection *c = &connections[i];
      if (c->fd < 0)
        continue;
      short ev = f[i + 1].revents;
      if (ev & (POLLERR | POLLNVAL)) {
        release(c);
        continue;
      }
      if (c->draining) {
        if (ev & (POLLIN | POLLHUP)) {
          unsigned char discard[512];
          ssize_t n = recv(c->fd, discard, sizeof discard, 0);
          if (n == 0 || (n < 0 && errno != EINTR && errno != EAGAIN &&
                         errno != EWOULDBLOCK)) {
            release(c);
            continue;
          }
        }
        if (now - c->drain_started >= 1000)
          release(c);
        continue;
      }
      if ((ev & (POLLIN | POLLHUP)) && !c->closing && c->count < 2 &&
          c->have == 0) {
        ssize_t n = recv(c->fd, c->input, sizeof c->input, 0);
        if (n > 0) {
          c->have = (size_t)n;
          c->used = 0;
          c->progress = now;
        } else if (n == 0) {
          c->eof = 1;
          if (c->parser.header_bytes || c->parser.request.body_length) {
            http_eof(&c->parser);
            if (enqueue(c, 0, now)) {
              release(c);
              continue;
            }
          }
          c->closing = 1;
        } else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
          release(c);
          continue;
        }
      }
      if (parse(c, now, authority)) {
        release(c);
        continue;
      }
      if (!c->closing && c->count < 2 &&
          http_expired(now, c->started, c->progress, 15000, 5000)) {
        c->parser.request.status = 408;
        c->parser.request.close = 1;
        if (enqueue(c, 0, now)) {
          release(c);
          continue;
        }
      }
      if (c->count) {
        struct edu_output *o = &c->output[c->head];
        if (now - o->started >= 15000) {
          release(c);
          continue;
        }
        if (ev & POLLOUT) {
          int r = http_drain(send_part, c, o->bytes, o->length, &o->offset);
          if (r < 0) {
            release(c);
            continue;
          }
          if (r == 1) {
            c->head = (c->head + 1) % 2;
            c->count--;
          }
        }
      }
      if (c->closing && !c->count) {
        shutdown(c->fd, SHUT_WR);
        c->draining = 1;
        c->drain_started = now;
      }
    }
  }
  for (unsigned i = 0; i < 4; i++)
    release(&connections[i]);
  close(listener);
  return 0;
}
