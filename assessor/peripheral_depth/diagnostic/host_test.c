#define _POSIX_C_SOURCE 200809L
#include "host.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("DG_HOST_CHECK:%d\n", __LINE__);                                  \
      rc = 1;                                                                  \
      goto done;                                                               \
    }                                                                          \
  } while (0)
static int receive_ready(int fd, DGSession *s, DGMessage *m) {
  struct pollfd p = {fd, POLLIN, 0};
  if (poll(&p, 1, 1000) != 1 || !(p.revents & POLLIN))
    return DG_IO;
  return dg_udp_receive(fd, s, m);
}
int main(void) {
  int fd[2] = {-1, -1}, rc = 0;
  struct sockaddr_in address[2];
  socklen_t size = sizeof address[0];
  for (unsigned i = 0; i < 2; ++i) {
    fd[i] = socket(AF_INET, SOCK_DGRAM, 0);
    CHECK(fd[i] >= 0);
    CHECK(fcntl(fd[i], F_SETFL, O_NONBLOCK) == 0);
    address[i] = (struct sockaddr_in){.sin_family = AF_INET,
                                      .sin_addr = {htonl(INADDR_LOOPBACK)},
                                      .sin_port = 0};
    CHECK(bind(fd[i], (struct sockaddr *)&address[i], sizeof address[i]) == 0);
    CHECK(getsockname(fd[i], (struct sockaddr *)&address[i], &size) == 0);
  }
  CHECK(connect(fd[0], (struct sockaddr *)&address[1], sizeof address[1]) == 0);
  CHECK(connect(fd[1], (struct sockaddr *)&address[0], sizeof address[0]) == 0);
  DGSession session = {0};
  DGMessage m = {1, 32, {0}}, out = {99, 0, {0}};
  uint8_t bytes[37];
  size_t n;
  CHECK(dg_udp_receive(fd[1], &session, &out) == DG_BUSY && !session.accepted);
  CHECK(dg_encode(&m, bytes, sizeof bytes, &n) == DG_OK && n == 36);
  bytes[36] = 23;
  CHECK(send(fd[0], bytes, 37, 0) == 37);
  CHECK(receive_ready(fd[1], &session, &out) == DG_FRAME && !session.accepted &&
        out.sequence == 99);
  CHECK(send(fd[0], bytes, 36, 0) == 36);
  CHECK(receive_ready(fd[1], &session, &out) == DG_OK &&
        session.accepted == 1 && out.sequence == 1);
  CHECK(send(fd[0], bytes, 36, 0) == 36);
  CHECK(receive_ready(fd[1], &session, &out) == DG_STALE &&
        session.accepted == 1);
  CHECK(send(fd[0], bytes, 0, 0) == 0);
  CHECK(receive_ready(fd[1], &session, &out) == DG_FRAME);
  CHECK(dg_udp_receive(fd[1], &session, &out) == DG_BUSY);
  puts("DG_HOST_PASS loopback_datagrams=4 truncated_prefix_rejected=1");
done:
  for (unsigned i = 0; i < 2; ++i)
    if (fd[i] >= 0 && close(fd[i]) != 0)
      rc = 1;
  return rc;
}
