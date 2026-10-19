#ifndef CM_EVENT_LOCAL_H
#define CM_EVENT_LOCAL_H
#include <sys/socket.h>
#include <time.h>
typedef struct {
  struct sockaddr_storage storage;
  socklen_t length;
} ElAddress;
typedef struct {
  int (*socket_fd)(void *, int, int, int);
  int (*bind_fd)(void *, int, const struct sockaddr *, socklen_t);
  int (*listen_fd)(void *, int, int);
  int (*connect_fd)(void *, int, const struct sockaddr *, socklen_t);
  int (*accept_fd)(void *, int);
  int (*close_fd)(void *, int);
  void *context;
} ElOps;
/* Only numeric loopback addresses; port0 is valid for listener allocation.
 * Every getaddrinfo list is freed before return. Output preserved on failure.
 */
int el_loopback(int family, unsigned port, ElAddress *out);
int el_pathname(const char *owned_private_path, ElAddress *out);
int el_listener(ElOps ops, const ElAddress *address, int *owned_fd);
/* All candidates must identify original owned local laboratory endpoints.
 * Each failed candidate is closed before the next socket is created.
 * The same absolute CLOCK_MONOTONIC deadline spans all candidates. */
int el_connect(ElOps ops, const ElAddress *candidates, unsigned count,
               struct timespec deadline, int *owned_fd, unsigned *error_checks);
/* Cancel callback borrows an accepted fd before publication; return1 to cancel,
 * 0 to publish. It may not close/transfer/retain the fd. Failed/cancelled calls
 * preserve output. Successful publication transfers exactly one close owner. */
int el_accept(ElOps ops, int listener, int (*cancel)(void *, int),
              void *context, int *owned_fd);
ElOps el_system_ops(void);
#endif
