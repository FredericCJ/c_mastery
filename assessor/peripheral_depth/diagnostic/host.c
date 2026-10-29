#define _POSIX_C_SOURCE 200809L
#include "host.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/uio.h>
#ifndef DG_HOST_MUTANT
#define DG_HOST_MUTANT 0
#endif
int dg_udp_receive(int fd, struct dg_session *s, struct dg_message *out)
{
    if (fd < 0 || !s || !out)
        return DG_INVALID;
    uint8_t bytes[36];
    struct iovec v = {bytes, sizeof bytes};
    struct msghdr msg = {0};
    msg.msg_iov = &v;
    msg.msg_iovlen = 1;
    ssize_t n = recvmsg(fd, &msg, 0);
    if (n < 0)
        return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR
                   ? DG_BUSY
                   : DG_IO;
    if (DG_HOST_MUTANT != 1 && (msg.msg_flags & MSG_TRUNC))
        return DG_FRAME;
    if ((size_t)n > sizeof bytes)
        return DG_FRAME;
    return dg_accept(s, bytes, (size_t)n, out);
}
