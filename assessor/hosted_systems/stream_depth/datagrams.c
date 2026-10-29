#include "stream.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define CHECK(c)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(c))                                                            \
        {                                                                    \
            fprintf(stderr, "DATAGRAM-CHECK:%s:%d errno=%d\n", #c, __LINE__, \
                    errno);                                                  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int descriptors(void)
{
    DIR *d = opendir("/proc/self/fd");
    if (!d)
        return -1;
    int count = 0;
    struct dirent *e;
    while ((e = readdir(d)))
        if (strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))
            ++count;
    return closedir(d) ? -1 : count;
}

static unsigned flags(int f)
{
    return ((f & MSG_TRUNC) ? RF_PAYLOAD_TRUNCATED : 0u) |
           ((f & MSG_CTRUNC) ? RF_CONTROL_TRUNCATED : 0u);
}

static int truncation(void)
{
    int fd[2];
    CHECK(socketpair(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                     fd) == 0);
    unsigned char input[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, out[8];
    CHECK(send(fd[0], input, 12, 0) == 12);
    struct iovec io = {out, sizeof out};
    struct msghdr msg = {.msg_iov = &io, .msg_iovlen = 1};
    ssize_t n = recvmsg(fd[1], &msg, 0);
    CHECK(n == 8 && (msg.msg_flags & MSG_TRUNC) &&
          !(msg.msg_flags & MSG_CTRUNC));
    CHECK(rf_datagram_accept((size_t)n, sizeof out, flags(msg.msg_flags)) ==
              0 &&
          !memcmp(out, input, 8));
    CHECK(send(fd[0], input, 12, 0) == 12);
    msg.msg_flags = 0;
    n = recvmsg(fd[1], &msg, MSG_TRUNC);
    CHECK(n == 12 && (msg.msg_flags & MSG_TRUNC) &&
          rf_datagram_accept((size_t)n, sizeof out, flags(msg.msg_flags)) == 0);
    CHECK(send(fd[0], input, 0, 0) == 0);
    msg.msg_flags = 0;
    n = recvmsg(fd[1], &msg, 0);
    CHECK(n == 0 && !msg.msg_flags &&
          rf_datagram_accept(0, sizeof out, 0) == 1);
    CHECK(recvmsg(fd[1], &msg, 0) == -1 &&
          (errno == EAGAIN || errno == EWOULDBLOCK));
    CHECK(close(fd[0]) == 0 && close(fd[1]) == 0);
    puts("DATAGRAM-TRUNCATION:sent12_buffer8_return8_flag=1;input_MSG_TRUNC_"
         "return12=1;zero_length_consumed=1;complete_frame_rejected=1;");
    return 0;
}

static int send_rights(int fd, int rights[2])
{
    union
    {
        struct cmsghdr align;
        unsigned char bytes[CMSG_SPACE(2 * sizeof(int))];
    } control = {0};

    char payload = 'C';
    struct iovec io = {&payload, 1};
    struct msghdr msg = {.msg_iov = &io,
                         .msg_iovlen = 1,
                         .msg_control = control.bytes,
                         .msg_controllen = sizeof control.bytes};
    struct cmsghdr *c = CMSG_FIRSTHDR(&msg);
    c->cmsg_level = SOL_SOCKET;
    c->cmsg_type = SCM_RIGHTS;
    c->cmsg_len = CMSG_LEN(2 * sizeof(int));
    memcpy(CMSG_DATA(c), rights, 2 * sizeof(int));
    return sendmsg(fd, &msg, 0) == 1 ? 0 : -1;
}

static int ancillary(void)
{
    int fd[2], pipefd[2];
    CHECK(socketpair(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                     fd) == 0 &&
          pipe2(pipefd, O_CLOEXEC) == 0);
    int before = descriptors();
    CHECK(before >= 4 && send_rights(fd[0], pipefd) == 0);
    unsigned char too_small[1] = {0};
    char payload = 0;
    struct iovec io = {&payload, 1};
    struct msghdr msg = {.msg_iov = &io,
                         .msg_iovlen = 1,
                         .msg_control = too_small,
                         .msg_controllen = sizeof too_small};
    CHECK(recvmsg(fd[1], &msg, MSG_CMSG_CLOEXEC) == 1 && payload == 'C' &&
          (msg.msg_flags & MSG_CTRUNC) && !(msg.msg_flags & MSG_TRUNC));
    CHECK(rf_datagram_accept(1, 1, flags(msg.msg_flags)) == 0 &&
          descriptors() == before);
    /* Complete control receives new owned references; close each exactly once.
     * Truncated excess descriptors above are closed by Linux, not exposed here.
     */
    CHECK(send_rights(fd[0], pipefd) == 0);

    union
    {
        struct cmsghdr align;
        unsigned char bytes[CMSG_SPACE(2 * sizeof(int))];
    } complete = {0};

    msg = (struct msghdr){.msg_iov = &io,
                          .msg_iovlen = 1,
                          .msg_control = complete.bytes,
                          .msg_controllen = sizeof complete.bytes};
    ssize_t received_bytes = recvmsg(fd[1], &msg, MSG_CMSG_CLOEXEC);
    printf("DATAGRAM-CONTROL-OBSERVATION:bytes=%zd;flags=%#x;\n",
           received_bytes, msg.msg_flags);
    CHECK(received_bytes == 1 && !(msg.msg_flags & (MSG_TRUNC | MSG_CTRUNC)) &&
          rf_datagram_accept(1, 1, 0) == 1);
    struct cmsghdr *c = CMSG_FIRSTHDR(&msg);
    CHECK(c && c->cmsg_level == SOL_SOCKET && c->cmsg_type == SCM_RIGHTS &&
          c->cmsg_len == CMSG_LEN(2 * sizeof(int)) &&
          CMSG_NXTHDR(&msg, c) == 0);
    int received[2];
    memcpy(received, CMSG_DATA(c), sizeof received);
    CHECK(received[0] != pipefd[0] && received[1] != pipefd[1] &&
          descriptors() == before + 2);
    for (unsigned i = 0; i < 2; ++i)
        CHECK((fcntl(received[i], F_GETFD) & FD_CLOEXEC) != 0 &&
              close(received[i]) == 0);
    CHECK(descriptors() == before);
    CHECK(close(fd[0]) == 0 && close(fd[1]) == 0 && close(pipefd[0]) == 0 &&
          close(pipefd[1]) == 0);
    puts("DATAGRAM-ANCILLARY:control_buffer1_CTRUNC=1;payload_complete_but_"
         "rejected=1;truncated_rights_no_leak=1;complete_rights_closed=2;");
    return 0;
}

static int acknowledgments(void)
{
    int fd[2];
    CHECK(socketpair(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                     fd) == 0);
    struct rf_dedup d;
    CHECK(rf_dedup_init(&d, 7) == 0);
    unsigned char request[3] = {7, 8, 3}, received[8], ack[6];
    /* Local send succeeds, but the teaching receiver discards before effect. */
    CHECK(send(fd[0], request, 3, 0) == 3 &&
          recv(fd[1], received, sizeof received, 0) == 3 && d.total == 0);
    CHECK(recv(fd[0], ack, sizeof ack, 0) == -1 &&
          (errno == EAGAIN || errno == EWOULDBLOCK));
    CHECK(send(fd[0], request, 3, 0) == 3 &&
          recv(fd[1], received, sizeof received, 0) == 3);
    unsigned char receipt[2] = {'R', received[1]};
    CHECK(send(fd[1], receipt, 2, 0) == 2 &&
          recv(fd[0], ack, sizeof ack, 0) == 2 && ack[0] == 'R' &&
          d.total == 0);
    uint32_t result;
    CHECK(rf_effect(&d, received[0], received[1], received[2], &result) == 1 &&
          result == 3);
    unsigned char effect[6] = {'E', received[1], 0,
                               0,   0,           (unsigned char)result};
    CHECK(send(fd[1], effect, 6, 0) == 6 &&
          recv(fd[0], ack, sizeof ack, 0) == 6 && ack[0] == 'E' && ack[5] == 3);
    /* Drop only the effect acknowledgment for id9, then replay same request. */
    request[1] = 9;
    request[2] = 5;
    CHECK(send(fd[0], request, 3, 0) == 3 &&
          recv(fd[1], received, sizeof received, 0) == 3);
    CHECK(rf_effect(&d, received[0], received[1], received[2], &result) == 1 &&
          result == 8);
    CHECK(recv(fd[0], ack, sizeof ack, 0) == -1 &&
          (errno == EAGAIN || errno == EWOULDBLOCK));
    CHECK(send(fd[0], request, 3, 0) == 3 &&
          recv(fd[1], received, sizeof received, 0) == 3);
    CHECK(rf_effect(&d, received[0], received[1], received[2], &result) == 0 &&
          result == 8 && d.total == 8);
    effect[1] = 9;
    effect[5] = (unsigned char)result;
    CHECK(send(fd[1], effect, 6, 0) == 6 &&
          recv(fd[0], ack, sizeof ack, 0) == 6 && ack[1] == 9 && ack[5] == 8);
    /* Original flawed teaching effect, same two attempts, no request cache. */
    unsigned flawed = 3;
    flawed += 5;
    flawed += 5;
    CHECK(flawed == 13 && flawed != d.total);
    CHECK(close(fd[0]) == 0 && close(fd[1]) == 0);
    puts("DATAGRAM-ACK:local_send_not_effect=1;receipt_before_effect=1;lost_"
         "effect_ack_replayed=1;dedup_total=8;flawed_total=13;durable_exactly_"
         "once_claim=0;");
    return 0;
}

int main(int argc, char **argv)
{
    alarm(8);
    CHECK(argc == 2);
    if (!strcmp(argv[1], "truncation"))
        return truncation();
    if (!strcmp(argv[1], "ancillary"))
        return ancillary();
    if (!strcmp(argv[1], "ack"))
        return acknowledgments();
    return 2;
}
