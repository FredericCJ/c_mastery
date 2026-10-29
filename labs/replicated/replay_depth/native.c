#define _POSIX_C_SOURCE 200809L
#include "replay_sink.h"
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

struct rp_file
{
    int fd;
    struct rpl_pages adapter;
    store pages;
};
static struct rp_file files[2];
static struct rpr_durable session;
static struct rpl_log inbox;
static const struct rp_state initial[2] = {{0, 0, 0, 1}, {0, 7, 0, 9}};

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

static int attach(unsigned index, int create)
{
    struct rp_file *f = &files[index];
    store_io io;
    CHECK(
        rpl_pages(&f->adapter,
                  (struct rpl_bytes){f, file_read, file_write, file_flush,
                                     STORE_IMAGE_PAGES * STORE_PAGE_SIZE, 4098},
                  &io) == RPL_OK);
    if (create)
    {
        CHECK(ftruncate(f->fd, STORE_IMAGE_PAGES * STORE_PAGE_SIZE) == 0 &&
              store_format(io) == 0);
    }
    CHECK(store_open(&f->pages, io) == 0);
    if (!index)
        CHECK((create ? rpr_create(&f->pages, 9, 4, initial, &session)
                      : rpr_open(&f->pages, 9, &session)) == 0);
    else
        CHECK((create
                   ? rpl_create(&f->pages, 9, RPR_EFFECT_SCHEMA, &inbox)
                   : rpl_open(&f->pages, 9, RPR_EFFECT_SCHEMA, &inbox)) == 0);
    return 0;
}

static int transfer(int fd, unsigned char *p, size_t n, int sending)
{
    size_t at = 0;
    unsigned calls = 0;
    while (at < n && calls++ < 200)
    {
        struct pollfd q = {fd, sending ? POLLOUT : POLLIN, 0};
        int r = poll(&q, 1, 1000);
        if (r < 0 && errno == EINTR)
            continue;
        if (r <= 0)
            return -1;
        size_t take = n - at;
        if (take > 3)
            take = 3;
        ssize_t z = sending ? send(fd, p + at, take, MSG_NOSIGNAL)
                            : recv(fd, p + at, take, 0);
        if (z < 0 && errno == EINTR)
            continue;
        if (z <= 0)
            return -1;
        at += (size_t)z;
    }
    return at == n ? 0 : -1;
}

static int sender(int fd)
{
    unsigned order[] = {2, 1, 4, 3, 2};
    int remote[] = {-1, 1, 0, -1};
    unsigned char barrier, wire[18];
    CHECK(transfer(fd, &barrier, 1, 0) == 0 && barrier == 1);
    for (unsigned i = 0; i < 5; ++i)
    {
        unsigned tick = order[i];
        CHECK(rpr_input_frame((struct rpr_input){9, tick, remote[tick - 1]},
                              wire) == 0 &&
              transfer(fd, wire, 18, 1) == 0);
    }
    CHECK(shutdown(fd, SHUT_WR) == 0);
    return 0;
}

static int process_case(void)
{
    int pair[2], status;
    pid_t child;
    struct rpr_notice n;
    struct rpr_effect effect, retry;
    unsigned applied;
    unsigned char b = 1, wire[18];
    CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == 0);
    child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        (void)close(pair[0]);
        int r = sender(pair[1]);
        (void)close(pair[1]);
        _exit(r ? 1 : 0);
    }
    CHECK(close(pair[1]) == 0);
    int local[] = {1, 0, -1, 1};
    for (unsigned i = 0; i < 4; ++i)
        CHECK(rpr_advance_durable(&session, local[i], &n) == 0);
    CHECK(rpr_prepare_durable(&session, &effect) == RPR_EMPTY);
    CHECK(transfer(pair[0], &b, 1, 1) == 0);
    for (unsigned i = 0; i < 5; ++i)
    {
        struct rpr_input input;
        CHECK(transfer(pair[0], wire, 18, 0) == 0 &&
              rpr_input_unframe(wire, 18, 2, &input) == 0 &&
              rpr_correct_durable(&session, input.timeline, input.tick,
                                  input.movement, &n) == 0);
    }
    CHECK(recv(pair[0], &b, 1, 0) == 0 && close(pair[0]) == 0 &&
          waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    CHECK(session.session.confirmed == 4 &&
          rpr_prepare_durable(&session, &effect) == 0 && effect.tick == 1);
    child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        if (attach(1, 0) || rpr_sink_apply(&inbox, effect, &applied) ||
            applied != 1)
            _exit(1);
        _exit(42);
    }
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 42);
    CHECK(attach(0, 0) == 0 && attach(1, 0) == 0 && inbox.memory.count == 1 &&
          rpr_prepare_durable(&session, &retry) == 0 &&
          retry.tick == effect.tick && retry.value == effect.value &&
          rpr_sink_apply(&inbox, retry, &applied) == 0 && applied == 0);
    CHECK(rpr_feedback_durable(&session, retry, AR_OK) == 0);
    struct rpr_effect changed = retry;
    changed.value += 2;
    applied = 77;
    CHECK(rpr_sink_apply(&inbox, changed, &applied) == RPR_CONFLICT &&
          applied == 77);
    changed = retry;
    changed.tick = 3;
    CHECK(rpr_sink_apply(&inbox, changed, &applied) == RPR_STALE &&
          applied == 77);
    for (unsigned tick = 2; tick <= 4; ++tick)
        CHECK(rpr_prepare_durable(&session, &effect) == 0 &&
              effect.tick == tick &&
              rpr_sink_apply(&inbox, effect, &applied) == 0 && applied == 1 &&
              rpr_feedback_durable(&session, effect, AR_OK) == 0);
    CHECK(attach(0, 0) == 0 && attach(1, 0) == 0 && inbox.memory.count == 4 &&
          session.session.effects.next == 4 &&
          !session.session.effects.pending &&
          rpr_prepare_durable(&session, &effect) == RPR_EMPTY);
    puts("REPLAY_NATIVE local-two-player corrections2-1-4-3-2 delayed-barrier "
         "partial-I/O durable-intents4 process-exit42 stable-retry");
    return 0;
}

int main(void)
{
    for (unsigned i = 0; i < 2; ++i)
    {
        char name[] = "/tmp/cmastery-replay-XXXXXX";
        files[i].fd = mkstemp(name);
        CHECK(files[i].fd >= 0 && unlink(name) == 0 && attach(i, 1) == 0);
    }
    CHECK(process_case() == 0);
    for (unsigned i = 0; i < 2; ++i)
        CHECK(close(files[i].fd) == 0);
    return 0;
}
