#ifndef CM_EVENT_LOCAL_H
#define CM_EVENT_LOCAL_H
#include <sys/socket.h>
#include <time.h>
struct el_address {
  struct sockaddr_storage storage;
  socklen_t length;
};
struct el_ops {
  int (*socket_fd)(void *, int, int, int);
  int (*bind_fd)(void *, int, const struct sockaddr *, socklen_t);
  int (*listen_fd)(void *, int, int);
  int (*connect_fd)(void *, int, const struct sockaddr *, socklen_t);
  int (*accept_fd)(void *, int);
  int (*close_fd)(void *, int);
  void *context;
};
/* Only numeric loopback addresses; port0 is valid for listener allocation.
 * Every getaddrinfo list is freed before return. Output preserved on failure.
 */
int el_loopback(int family, unsigned port, struct el_address *out);
int el_pathname(const char *owned_private_path, struct el_address *out);
int el_listener(struct el_ops ops, const struct el_address *address, int *owned_fd);
/* All candidates must identify original owned local laboratory endpoints.
 * Each failed candidate is closed before the next socket is created.
 * The same absolute CLOCK_MONOTONIC deadline spans all candidates. */
int el_connect(struct el_ops ops, const struct el_address *candidates, unsigned count,
               struct timespec deadline, int *owned_fd, unsigned *error_checks);
/* Cancel callback borrows an accepted fd before publication; return1 to cancel,
 * 0 to publish. It may not close/transfer/retain the fd. Failed/cancelled calls
 * preserve output. Successful publication transfers exactly one close owner. */
int el_accept(struct el_ops ops, int listener, int (*cancel)(void *, int),
              void *context, int *owned_fd);
struct el_ops el_system_ops(void);
#endif
