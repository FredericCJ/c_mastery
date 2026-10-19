#define _POSIX_C_SOURCE 200809L
#include "host.h"
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("DG_PUBLIC_HOST_CHECK:%d\n", __LINE__);                           \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  int fd[2];
  CHECK(socketpair(AF_UNIX, SOCK_DGRAM, 0, fd) == 0);
  CHECK(fcntl(fd[1], F_SETFL, O_NONBLOCK) == 0);
  DGSession s = {0};
  DGMessage m = {0};
  const uint8_t bytes[6] = {0, 1, 0, 2, 1, 2};
  CHECK(dg_udp_receive(fd[1], &s, &m) == DG_BUSY);
  CHECK(send(fd[0], bytes, 6, 0) == 6);
  struct pollfd ready = {fd[1], POLLIN, 0};
  CHECK(poll(&ready, 1, 1000) == 1 && (ready.revents & POLLIN));
  CHECK(dg_udp_receive(fd[1], &s, &m) == DG_OK && m.sequence == 1 &&
        m.length == 2);
  CHECK(close(fd[0]) == 0 && close(fd[1]) == 0);
  puts("DG_PUBLIC_HOST_PASS local_datagram_API_only");
  return 0;
}
