#include "local.h"
#include "signals.h"
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
static int socket_fn(void *c, int family, int type, int protocol) {
  (void)c;
  return socket(family, type, protocol);
}
static int bind_fn(void *c, int fd, const struct sockaddr *a, socklen_t n) {
  (void)c;
  return bind(fd, a, n);
}
static int listen_fn(void *c, int fd, int n) {
  (void)c;
  return listen(fd, n);
}
static int connect_fn(void *c, int fd, const struct sockaddr *a, socklen_t n) {
  (void)c;
  return connect(fd, a, n);
}
static int accept_fn(void *c, int fd) {
  (void)c;
  return accept4(fd, 0, 0, SOCK_NONBLOCK | SOCK_CLOEXEC);
}
static int close_fn(void *c, int fd) {
  (void)c;
  return close(fd);
}
ElOps el_system_ops(void) {
  return (ElOps){socket_fn, bind_fn,  listen_fn, connect_fn,
                 accept_fn, close_fn, 0};
}
static int ops_valid(ElOps o) {
  return o.socket_fd && o.bind_fd && o.listen_fd && o.connect_fd &&
         o.accept_fd && o.close_fd;
}
static int address_valid(const ElAddress *a) {
  if (!a)
    return 0;
  if (a->storage.ss_family == AF_INET) {
    if (a->length != sizeof(struct sockaddr_in))
      return 0;
    struct sockaddr_in v;
    memcpy(&v, &a->storage, sizeof v);
    return ntohl(v.sin_addr.s_addr) == INADDR_LOOPBACK;
  }
  if (a->storage.ss_family == AF_INET6) {
    if (a->length != sizeof(struct sockaddr_in6))
      return 0;
    struct sockaddr_in6 v;
    memcpy(&v, &a->storage, sizeof v);
    return IN6_IS_ADDR_LOOPBACK(&v.sin6_addr);
  }
  if (a->storage.ss_family == AF_UNIX) {
    struct sockaddr_un v = {0};
    const struct sockaddr_un *u = &v;
    size_t base = offsetof(struct sockaddr_un, sun_path);
    if (a->length <= base || a->length > sizeof v)
      return 0;
    memcpy(&v, &a->storage, a->length);
    if (u->sun_path[0] != '/')
      return 0;
    size_t n = a->length - base;
    return u->sun_path[n - 1] == '\0' && strnlen(u->sun_path, n) == n - 1;
  }
  return 0;
}
int el_loopback(int family, unsigned port, ElAddress *out) {
  if (!out || port > 65535 || (family != AF_INET && family != AF_INET6)) {
    errno = EAFNOSUPPORT;
    return -1;
  }
  struct addrinfo hints = {0}, *list = 0;
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
  char service[6];
  snprintf(service, sizeof service, "%u", port);
  int e = getaddrinfo(family == AF_INET ? "127.0.0.1" : "::1", service, &hints,
                      &list);
  if (e) {
    errno = e == EAI_MEMORY ? ENOMEM : EINVAL;
    return -1;
  }
  ElAddress next = {0};
  int found = 0;
  for (struct addrinfo *r = list; r; r = r->ai_next) {
    if (r->ai_family != family || r->ai_addrlen > sizeof next.storage)
      continue;
    memcpy(&next.storage, r->ai_addr, r->ai_addrlen);
    next.length = r->ai_addrlen;
    if (address_valid(&next)) {
      found = 1;
      break;
    }
  }
  freeaddrinfo(list);
  if (!found) {
    errno = EAFNOSUPPORT;
    return -1;
  }
  *out = next;
  return 0;
}
int el_pathname(const char *path, ElAddress *out) {
  if (!path || !out || path[0] != '/') {
    errno = EINVAL;
    return -1;
  }
  struct sockaddr_un u = {0};
  size_t n = strnlen(path, sizeof u.sun_path);
  if (n == sizeof u.sun_path) {
    errno = ENAMETOOLONG;
    return -1;
  }
  u.sun_family = AF_UNIX;
  memcpy(u.sun_path, path, n + 1);
  ElAddress next = {0};
  next.length = (socklen_t)(offsetof(struct sockaddr_un, sun_path) + n + 1);
  memcpy(&next.storage, &u, next.length);
  *out = next;
  return 0;
}
int el_listener(ElOps o, const ElAddress *a, int *out) {
  if (!ops_valid(o) || !address_valid(a) || !out) {
    errno = EINVAL;
    return -1;
  }
  int fd = o.socket_fd(o.context, a->storage.ss_family,
                       SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (fd < 0)
    return -1;
  if (o.bind_fd(o.context, fd, (const struct sockaddr *)&a->storage,
                a->length) ||
      o.listen_fd(o.context, fd, 4)) {
    int e = errno;
    o.close_fd(o.context, fd);
    errno = e;
    return -1;
  }
  *out = fd;
  return 0;
}
int el_connect(ElOps o, const ElAddress *a, unsigned count,
               struct timespec deadline, int *out, unsigned *checks) {
  if (!ops_valid(o) || !a || !out || !checks || !count || count > 8) {
    errno = EINVAL;
    return -1;
  }
  unsigned inspected = 0;
  int final = ECONNREFUSED;
  for (unsigned i = 0; i < count; ++i) {
    if (!address_valid(&a[i])) {
      final = EAFNOSUPPORT;
      continue;
    }
    struct timespec now, left;
    if (clock_gettime(CLOCK_MONOTONIC, &now))
      return -1;
    int remains = es_remaining(now, deadline, &left);
    if (remains <= 0) {
      errno = remains == 0 ? ETIMEDOUT : errno;
      *checks = inspected;
      return -1;
    }
    int fd = o.socket_fd(o.context, a[i].storage.ss_family,
                         SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) {
      final = errno;
      continue;
    }
    int r = o.connect_fd(o.context, fd, (const struct sockaddr *)&a[i].storage,
                         a[i].length);
    int error = r == 0 ? 0 : errno;
    if (r < 0 && error == EINPROGRESS) {
      for (;;) {
        if (clock_gettime(CLOCK_MONOTONIC, &now)) {
          error = errno;
          break;
        }
        remains = es_remaining(now, deadline, &left);
        if (remains <= 0) {
          error = remains == 0 ? ETIMEDOUT : errno;
          break;
        }
        struct pollfd p = {fd, POLLOUT, 0};
        r = ppoll(&p, 1, &left, 0);
        if (r < 0 && errno == EINTR)
          continue;
        if (r < 0) {
          error = errno;
          break;
        }
        if (!r) {
          error = ETIMEDOUT;
          break;
        }
        socklen_t length = sizeof error;
        error = 0;
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length)) {
          error = errno;
          break;
        }
        ++inspected;
        if (length != sizeof error) {
          error = EIO;
          break;
        }
#ifdef EL_MUTANT_WRITABLE_SUCCESS
        error = 0;
#endif
        break;
      }
    }
    if (!error) {
      *checks = inspected;
      *out = fd;
      return 0;
    }
    final = error;
    o.close_fd(o.context, fd);
    if (error == ETIMEDOUT)
      break;
  }
  *checks = inspected;
  errno = final;
  return -1;
}
int el_accept(ElOps o, int listener, int (*cancel)(void *, int), void *context,
              int *out) {
  if (!ops_valid(o) || listener < 0 || !out) {
    errno = EINVAL;
    return -1;
  }
  int fd = o.accept_fd(o.context, listener);
  if (fd < 0)
    return -1;
  if (cancel && cancel(context, fd)) {
#ifndef EL_MUTANT_CANCEL_LEAK
    o.close_fd(o.context, fd);
#endif
    errno = ECANCELED;
    return -1;
  }
  *out = fd;
  return 0;
}
