#define _POSIX_C_SOURCE 200809L
#include "test_support.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

struct rp_file
{
    int fd;
    struct rpl_pages adapter;
    store pages;
    struct rpl_log log;
};
static struct rp_file files[3];

static ptrdiff_t file_read(void *c, size_t at, unsigned char *p, size_t n)
{
    struct rp_file *f = c;
    if (n > 113)
        n = 113;
    ssize_t r = pread(f->fd, p, n, (off_t)at);
    return r < 0 && errno == EINTR ? -2 : r;
}

static ptrdiff_t file_write(void *c, size_t at, const unsigned char *p,
                            size_t n)
{
    struct rp_file *f = c;
    if (n > 257)
        n = 257;
    ssize_t r = pwrite(f->fd, p, n, (off_t)at);
    return r < 0 && errno == EINTR ? -2 : r;
}

static int file_flush(void *c)
{
    return fdatasync(((struct rp_file *)c)->fd);
}

static int attach(struct rp_file *f, unsigned schema, int create)
{
    store_io io;
    CHECK(
        rpl_pages(&f->adapter,
                  (struct rpl_bytes){f, file_read, file_write, file_flush,
                                     STORE_IMAGE_PAGES * STORE_PAGE_SIZE, 4098},
                  &io) == RPL_OK);
    if (create)
    {
        CHECK(!ftruncate(f->fd, STORE_IMAGE_PAGES * STORE_PAGE_SIZE));
        CHECK(store_format(io) == STORE_OK);
    }
    CHECK(store_open(&f->pages, io) == STORE_OK);
    CHECK((create ? rpl_create(&f->pages, 19, schema, &f->log)
                  : rpl_open(&f->pages, 19, schema, &f->log)) == RPL_OK);
    return 0;
}

static int socket_write(void *c, const unsigned char *p, size_t n)
{
    int fd = *(int *)c;
    struct pollfd q = {fd, POLLOUT, 0};
    int r = poll(&q, 1, 1000);
    if (r < 0 && errno == EINTR)
        return 0;
    if (r <= 0)
        return -1;
    if (n > 3)
        n = 3;
    ssize_t sent = send(fd, p, n, MSG_NOSIGNAL);
    if (sent < 0 && errno == EINTR)
        return 0;
    return (int)sent;
}

static ptrdiff_t socket_read(void *c, unsigned char *p, size_t n)
{
    int fd = *(int *)c;
    struct pollfd q = {fd, POLLIN, 0};
    int r = poll(&q, 1, 1000);
    if (r < 0 && errno == EINTR)
        return -2;
    if (r <= 0)
        return -1;
    if (n > 2)
        n = 2;
    ssize_t got = recv(fd, p, n, 0);
    return got < 0 && errno == EINTR ? -2 : got;
}

static int receiver_child(int fd, unsigned cut)
{
    CHECK(!attach(&files[2], 7, 0));
    struct rpo_receiver receiver;
    CHECK(rpo_receiver(&files[2].log, &receiver) == RPL_OK);
    unsigned char request[64], response[24];
    CHECK(rpo_wire_read(socket_read, &fd, 200, 64, request) == RPL_OK);
    struct rpl_event event;
    CHECK(rpl_event_decode(19, 7, request, &event) == RPL_OK);
    struct rpo_reply reply;
    int r = rpo_receive(&receiver, 19, 7, event, &reply);
    CHECK(r == RPL_OK || r == RPO_REPLAY);
    if (cut == 1)
        _exit(42); /* Actual process termination after durable inbox commit. */
    CHECK(rpo_reply_encode(reply, response) == RPL_OK);
    CHECK(rpo_wire_write(socket_write, &fd, 200, response, 24) == RPL_OK);
    CHECK(!shutdown(fd, SHUT_WR));
    CHECK(!close(fd));
    return 0;
}

static int exchange(unsigned cut, int acknowledge)
{
    struct rpo_sender sender;
    CHECK(rpo_sender(&files[0].log, &files[1].log, &sender) == RPL_OK);
    struct rpl_event event;
    CHECK(rpo_next(&sender, &event) == RPL_OK);
    unsigned char request[64], response[24];
    CHECK(rpl_event_encode(19, 7, event, request) == RPL_OK);
    int sockets[2];
    CHECK(!socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        (void)close(sockets[0]);
        int status = receiver_child(sockets[1], cut);
        _exit(status);
    }
    CHECK(!close(sockets[1]));
    CHECK(rpo_wire_write(socket_write, &sockets[0], 200, request, 64) ==
          RPL_OK);
    CHECK(!shutdown(sockets[0], SHUT_WR));
    int r = rpo_wire_read(socket_read, &sockets[0], 200, 24, response);
    CHECK(!close(sockets[0]));
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status));
    if (cut == 1)
    {
        CHECK(WEXITSTATUS(status) == 42 && r == RPL_CORRUPT);
        return 0;
    }
    CHECK(WEXITSTATUS(status) == 0 && r == RPL_OK);
    struct rpo_reply reply;
    CHECK(rpo_reply_decode(response, &reply) == RPL_OK);
    if (acknowledge)
        CHECK(rpo_ack(&sender, reply) == RPL_OK);
    return 0;
}

static int native_case(void)
{
    for (unsigned i = 0; i < 3; ++i)
    {
        char path[] = "/tmp/cmastery-rpo-XXXXXX";
        files[i].fd = mkstemp(path);
        CHECK(files[i].fd >= 0);
        CHECK(!unlink(path));
        CHECK(!attach(&files[i], i == 1 ? RPO_ACK_SCHEMA : 7, 1));
    }
    CHECK(rpl_append(&files[0].log, 1, 2, 7) == RPL_OK);
    /* Source process dies after commit and before sending: parent recovers it. */
    pid_t process = fork();
    CHECK(process >= 0);
    if (!process)
    {
        CHECK(!attach(&files[0], 7, 0));
        CHECK(rpl_append(&files[0].log, 1, 3, -2) == RPL_OK);
        _exit(43);
    }
    int status;
    CHECK(waitpid(process, &status, 0) == process && WIFEXITED(status) &&
          WEXITSTATUS(status) == 43);
    CHECK(!attach(&files[0], 7, 0));
    CHECK(files[0].log.memory.count == 2);
    CHECK(!exchange(1, 0));
    CHECK(!attach(&files[2], 7, 0));
    CHECK(files[2].log.memory.count == 1 && files[1].log.memory.count == 0);
    /* Sender obtains a valid reply, then terminates before durable ACK. */
    process = fork();
    CHECK(process >= 0);
    if (!process)
    {
        int r = exchange(0, 0);
        _exit(r ? 1 : 44);
    }
    CHECK(waitpid(process, &status, 0) == process && WIFEXITED(status) &&
          WEXITSTATUS(status) == 44);
    CHECK(!attach(&files[1], RPO_ACK_SCHEMA, 0));
    CHECK(files[1].log.memory.count == 0);
    CHECK(!exchange(0, 1));
    CHECK(!exchange(0, 1));
    CHECK(!attach(&files[2], 7, 0));
    CHECK(files[2].log.memory.count == 2 && files[1].log.memory.count == 2);
    int64_t total = 0;
    CHECK(rpo_projection(&files[2].log, 2, &total) == RPL_OK && total == 5);
    /* Original deliberately flawed external-effect protocol. The effect and
     * inbox have separate commit authorities; receiver death exposes the gap. */
    char path[] = "/tmp/cmastery-rpo-effect-XXXXXX";
    int effect = mkstemp(path);
    CHECK(effect >= 0);
    CHECK(!unlink(path));
    unsigned char zero = 0;
    CHECK(pwrite(effect, &zero, 1, 0) == 1 && !fdatasync(effect));
    process = fork();
    CHECK(process >= 0);
    if (!process)
    {
        unsigned char n;
        CHECK(pread(effect, &n, 1, 0) == 1);
        ++n;
        CHECK(pwrite(effect, &n, 1, 0) == 1 && !fdatasync(effect));
        _exit(45);
    }
    CHECK(waitpid(process, &status, 0) == process && WIFEXITED(status) &&
          WEXITSTATUS(status) == 45);
    unsigned char n;
    CHECK(pread(effect, &n, 1, 0) == 1 && n == 1);
    ++n;
    CHECK(pwrite(effect, &n, 1, 0) == 1 && !fdatasync(effect));
    CHECK(pread(effect, &n, 1, 0) == 1 && n == 2);
    CHECK(!close(effect));
    for (unsigned i = 0; i < 3; ++i)
        CHECK(!close(files[i].fd));
    puts(
        "OUTBOX_NATIVE source-death receiver-death sender-death dedup=2 total=5 "
        "external-duplicate=2");
    return 0;
}

int main(void)
{
    return native_case();
}
