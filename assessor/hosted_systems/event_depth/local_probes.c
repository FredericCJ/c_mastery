#include "local.h"
#include "signals.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
struct rd_slot {
  int fd, live;
};
struct rd_ledger {
  struct rd_slot slots[32];
  unsigned used, live, closed, bad, fail;
  struct el_ops system;
};
enum {
  FAIL_NONE,
  FAIL_SOCKET,
  FAIL_BIND,
  FAIL_LISTEN,
  FAIL_CONNECT,
  FAIL_ACCEPT
};
static int inject(struct rd_ledger *l, unsigned op) {
  if (l->fail != op)
    return 0;
  l->fail = 0;
  errno = EIO;
  return 1;
}
static int track(struct rd_ledger *l, int fd) {
  if (fd >= 0) {
    if (l->used == 32) {
      l->system.close_fd(0, fd);
      errno = EMFILE;
      return -1;
    }
    l->slots[l->used++] = (struct rd_slot){fd, 1};
    ++l->live;
  }
  return fd;
}
static int new_socket(void *p, int family, int type, int protocol) {
  struct rd_ledger *l = p;
  if (inject(l, FAIL_SOCKET))
    return -1;
  return track(l, l->system.socket_fd(0, family, type, protocol));
}
static int bind_socket(void *p, int fd, const struct sockaddr *a, socklen_t n) {
  struct rd_ledger *l = p;
  return inject(l, FAIL_BIND) ? -1 : l->system.bind_fd(0, fd, a, n);
}
static int listen_socket(void *p, int fd, int n) {
  struct rd_ledger *l = p;
  return inject(l, FAIL_LISTEN) ? -1 : l->system.listen_fd(0, fd, n);
}
static int connect_socket(void *p, int fd, const struct sockaddr *a,
                          socklen_t n) {
  struct rd_ledger *l = p;
  return inject(l, FAIL_CONNECT) ? -1 : l->system.connect_fd(0, fd, a, n);
}
static int accept_socket(void *p, int fd) {
  struct rd_ledger *l = p;
  return inject(l, FAIL_ACCEPT) ? -1 : track(l, l->system.accept_fd(0, fd));
}
static int close_socket(void *p, int fd) {
  struct rd_ledger *l = p;
  for (unsigned i = 0; i < l->used; ++i)
    if (l->slots[i].fd == fd && l->slots[i].live) {
      l->slots[i].live = 0;
      --l->live;
      ++l->closed;
      return l->system.close_fd(0, fd);
    }
  ++l->bad;
  errno = EBADF;
  return -1;
}
static struct el_ops operations(struct rd_ledger *l) {
  l->system = el_system_ops();
  return (struct el_ops){new_socket,
                 bind_socket,
                 listen_socket,
                 connect_socket,
                 accept_socket,
                 close_socket,
                 l};
}
static struct timespec deadline(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  t.tv_sec += 2;
  return t;
}
static int bound_address(int fd, struct el_address *a) {
  a->length = sizeof a->storage;
  return getsockname(fd, (struct sockaddr *)&a->storage, &a->length);
}
static int cancel(void *p, int fd) {
  unsigned *calls = p;
  ++*calls;
  return fcntl(fd, F_GETFD) >= 0;
}
static int accept_ready(struct el_ops o, int listener, int (*on_cancel)(void *, int),
                        void *context, int *out) {
  struct timespec end = deadline();
  for (;;) {
    int r = el_accept(o, listener, on_cancel, context, out);
    if (r == 0)
      return 0;
    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
      return -1;
    struct timespec now, left;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (es_remaining(now, end, &left) <= 0) {
      errno = ETIMEDOUT;
      return -1;
    }
    struct pollfd p = {listener, POLLIN, 0};
    r = ppoll(&p, 1, &left, 0);
    if (r < 0 && errno != EINTR)
      return -1;
  }
}
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c)) {                                                                \
      fprintf(stderr, "LOCAL-PROBE-CHECK:%s:%d errno=%d\n", #c, __LINE__,      \
              errno);                                                          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int candidates(void) {
  struct rd_ledger l = {0};
  struct el_ops o = operations(&l);
  struct el_address addresses[2], zero;
  CHECK(el_loopback(AF_INET, 0, &zero) == 0);
  int reserved =
      o.socket_fd(&l, AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  CHECK(reserved >= 0);
  CHECK(o.bind_fd(&l, reserved, (const struct sockaddr *)&zero.storage,
                  zero.length) == 0);
  CHECK(bound_address(reserved, &addresses[0]) == 0);
  int listener = -1;
  CHECK(el_listener(o, &zero, &listener) == 0);
  CHECK(bound_address(listener, &addresses[1]) == 0);
  /* Resolve the two owned ephemeral ports numerically, freeing each real list.
   * The first remains bound but deliberately not listening, preventing
   * takeover. */
  for (unsigned i = 0; i < 2; ++i) {
    struct sockaddr_in a;
    memcpy(&a, &addresses[i].storage, sizeof a);
    CHECK(el_loopback(AF_INET, ntohs(a.sin_port), &addresses[i]) == 0);
  }
  int connected = -1;
  unsigned checks = 0;
  CHECK(el_connect(o, addresses, 2, deadline(), &connected, &checks) == 0 &&
        connected >= 0 && checks >= 1);
  struct sockaddr_storage peer;
  socklen_t n = sizeof peer;
  CHECK(getpeername(connected, (struct sockaddr *)&peer, &n) == 0);
  struct sockaddr_in target;
  memcpy(&target, &addresses[1].storage, sizeof target);
  struct sockaddr_in actual;
  memcpy(&actual, &peer, sizeof actual);
  CHECK(actual.sin_port == target.sin_port);
  int accepted = -1;
  CHECK(accept_ready(o, listener, 0, 0, &accepted) == 0);
  CHECK(write(connected, "X", 1) == 1);
  char byte;
  struct timespec read_deadline = deadline();
  for (;;) {
    ssize_t got = read(accepted, &byte, 1);
    if (got == 1)
      break;
    CHECK(got < 0 &&
          (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR));
    CHECK(es_wait_fd(accepted, read_deadline) > 0);
  }
  CHECK(byte == 'X');
  CHECK(o.close_fd(&l, accepted) == 0 && o.close_fd(&l, connected) == 0 &&
        o.close_fd(&l, listener) == 0 && o.close_fd(&l, reserved) == 0);
  CHECK(l.live == 0 && l.bad == 0 && l.closed == l.used);
  printf("LOCAL-CANDIDATES:attempts=2;so_error_checks=%u;created=%u;closed=%u;"
         "peer_is_second=1;\n",
         checks, l.used, l.closed);
  return 0;
}
static int failures(void) {
  for (unsigned fault = FAIL_SOCKET; fault <= FAIL_ACCEPT; ++fault) {
    struct rd_ledger l = {0};
    struct el_ops o = operations(&l);
    struct el_address address;
    CHECK(el_loopback(AF_INET, 0, &address) == 0);
    int listener = -1;
    if (fault <= FAIL_LISTEN) {
      l.fail = fault;
      CHECK(el_listener(o, &address, &listener) == -1 && errno == EIO &&
            listener == -1);
    } else {
      CHECK(el_listener(o, &address, &listener) == 0);
      CHECK(bound_address(listener, &address) == 0);
      l.fail = fault;
      int connection = -1;
      unsigned checks = 0;
      int r = el_connect(o, &address, 1, deadline(), &connection, &checks);
      if (fault == FAIL_CONNECT)
        CHECK(r == -1 && errno == EIO && connection == -1);
      else {
        CHECK(r == 0);
        int accepted = -1;
        CHECK(accept_ready(o, listener, 0, 0, &accepted) == -1 &&
              errno == EIO && accepted == -1);
        CHECK(o.close_fd(&l, connection) == 0);
      }
      CHECK(o.close_fd(&l, listener) == 0);
    }
    CHECK(l.live == 0 && l.bad == 0 && l.closed == l.used && !l.fail);
  }
  struct rd_ledger l = {0};
  struct el_ops o = operations(&l);
  struct el_address address;
  CHECK(el_loopback(AF_INET, 0, &address) == 0);
  int listener = -1, connection = -1, accepted = -1;
  unsigned checked = 0, calls = 0;
  CHECK(el_listener(o, &address, &listener) == 0 &&
        bound_address(listener, &address) == 0 &&
        el_connect(o, &address, 1, deadline(), &connection, &checked) == 0);
  CHECK(accept_ready(o, listener, cancel, &calls, &accepted) == -1 &&
        errno == ECANCELED && accepted == -1 && calls == 1);
  CHECK(l.live == 2);
  CHECK(o.close_fd(&l, connection) == 0 && o.close_fd(&l, listener) == 0 &&
        l.live == 0 && l.bad == 0 && l.used == l.closed);
  puts("LOCAL-FAILURES:socket_bind_listen_connect_accept=5;cancel_after_accept="
       "1;closed_once=1;ledger_restored=6;");
  return 0;
}
static int families(const char *path) {
  for (unsigned i = 0; i < 2; ++i) {
    struct rd_ledger l = {0};
    struct el_ops o = operations(&l);
    struct el_address a;
    CHECK((i ? el_pathname(path, &a) : el_loopback(AF_INET6, 0, &a)) == 0);
    int listener = -1, connection = -1, accepted = -1;
    unsigned checked = 0;
    CHECK(el_listener(o, &a, &listener) == 0);
    if (!i)
      CHECK(bound_address(listener, &a) == 0);
    CHECK(el_connect(o, &a, 1, deadline(), &connection, &checked) == 0 &&
          accept_ready(o, listener, 0, 0, &accepted) == 0);
    CHECK(o.close_fd(&l, accepted) == 0 && o.close_fd(&l, connection) == 0 &&
          o.close_fd(&l, listener) == 0 && l.live == 0 && l.bad == 0);
    if (i)
      CHECK(unlink(path) == 0);
  }
  puts("LOCAL-FAMILIES:actual_ipv6_loopback=1;actual_private_unix=1;all_"
       "handles_closed=1;");
  return 0;
}
int main(int argc, char **argv) {
  alarm(10);
  CHECK(argc >= 2);
  if (!strcmp(argv[1], "candidates"))
    return candidates();
  if (!strcmp(argv[1], "failures"))
    return failures();
  if (!strcmp(argv[1], "families") && argc == 3)
    return families(argv[2]);
  return 2;
}
