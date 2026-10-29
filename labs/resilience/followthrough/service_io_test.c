#include "service_io.h"
#include "journal_posix.h"
#include "hosted.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#define CHECK(c)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(c))                                                            \
        {                                                                    \
            fprintf(stderr, "RZ_CHECK: %s:%d %s\n", __FILE__, __LINE__, #c); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

struct rz_fixture
{
    int lf, af;
    struct rzj_posix l, a;
    struct rzj_journal journal;
    struct msg_hub hub;
    struct rzs_outbox outbox;
    struct rzv_service service;
    atomic_uint released;
};

struct rz_worker
{
    struct rzv_service *service;
    struct rzf_scope *scope;
    int fd, cancel, done;
    size_t chunk;
    struct rzv_observation observation;
    int status;
};

static void *allocate(void *p, size_t n)
{
    (void)p;
    return malloc(n);
}

static void deallocate(void *p, void *v)
{
    atomic_fetch_add((atomic_uint *)p, 1);
    free(v);
}

static uint64_t clock_ms(void)
{
    uint64_t now = 0;
    CHECK(rzv_now(&now) == RZ_OK);
    return now;
}

static struct rzf_scope *scope(struct rz_fixture *f, uint64_t budget)
{
    struct rzf_scope *s = NULL;
    CHECK(rzf_scope_create(
              NULL, clock_ms(), budget,
              (struct rzf_allocator){&f->released, allocate, deallocate},
              &s) == RZ_OK);
    return s;
}

static void initialize(struct rz_fixture *f, unsigned capacity)
{
    *f = (struct rz_fixture){0};
    atomic_init(&f->released, 0);
    char lp[] = "/tmp/cmastery-rzv-log-XXXXXX",
         ap[] = "/tmp/cmastery-rzv-ack-XXXXXX";
    f->lf = mkstemp(lp);
    f->af = mkstemp(ap);
    CHECK(f->lf >= 0 && f->af >= 0 && !unlink(lp) && !unlink(ap));
    struct rzj_io li, ai;
    CHECK(rzj_posix(f->lf, 3, &f->l, &li) == RZ_OK &&
          rzj_posix(f->af, 3, &f->a, &ai) == RZ_OK);
    CHECK(rzj_open(li, 16, &f->journal) == RZ_OK);
    uint64_t epoch;
    CHECK(rzj_issue(&f->journal, clock_ms() / 1000, &epoch) == RZ_OK);
    CHECK(msg_hub_init(&f->hub, 1, 1) == MSG_OK &&
          rzs_open(&f->journal, ai, &f->hub, &f->outbox) == RZ_OK);
    CHECK(rzv_init(&f->service, &f->outbox, capacity, clock_ms()) == RZ_OK);
}

static void destroy(struct rz_fixture *f)
{
    CHECK(rzv_destroy(&f->service) == RZ_OK);
    msg_hub_destroy(&f->hub);
    CHECK(!close(f->lf) && !close(f->af));
}

static void pair(int sockets[2], int cancel[2])
{
    CHECK(!socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                      sockets));
    CHECK(!pipe(cancel));
}

static void close_pair(int sockets[2], int cancel[2])
{
    for (unsigned i = 0; i < 2; ++i)
    {
        CHECK(!close(sockets[i]));
        CHECK(!close(cancel[i]));
    }
}

static void *worker(void *p)
{
    struct rz_worker *w = p;
    w->status = rzv_serve(w->service, w->scope, w->fd, w->cancel, w->chunk,
                          &w->observation);
    if (w->done >= 0)
    {
        unsigned char one = 1;
        CHECK(write(w->done, &one, 1) == 1);
    }
    return NULL;
}

static void send_request(int fd, struct rzj_request request)
{
    unsigned char b[RZV_FRAME_BYTES];
    CHECK(rzv_request_encode(request, b) == RZ_OK);
    for (size_t i = 0; i < sizeof b; ++i)
    {
        CHECK(send(fd, b + i, 1, MSG_NOSIGNAL) == 1);
    }
}

static struct rzv_reply response(int fd)
{
    unsigned char b[RZV_FRAME_BYTES];
    size_t used = 0;
    while (used < sizeof b)
    {
        struct pollfd p = {fd, POLLIN, 0};
        CHECK(poll(&p, 1, 2000) > 0);
        ssize_t n = read(fd, b + used, 1);
        CHECK(n == 1);
        used += (size_t)n;
    }
    struct rzv_reply reply = {0};
    CHECK(rzv_reply_decode(b, &reply) == RZ_OK);
    return reply;
}

static void parser(void)
{
    unsigned char bytes[2 * RZV_FRAME_BYTES];
    CHECK(rzv_request_encode((struct rzj_request){1, 15, UINT64_MAX, -8},
                             bytes) == RZ_OK);
    for (unsigned i = 0; i < RZV_FRAME_BYTES; ++i)
    {
        bytes[i + RZV_FRAME_BYTES] = bytes[i];
    }
    for (size_t split = 0; split <= RZV_FRAME_BYTES; ++split)
    {
        struct rzv_frame frame = {0};
        struct rzj_request request = {0};
        size_t used = 99;
        CHECK(rzv_frame_feed(&frame, bytes, split, &used, &request) ==
                  (split == RZV_FRAME_BYTES ? RZ_OK : RZ_PENDING) &&
              used == split);
        if (split < RZV_FRAME_BYTES)
        {
            CHECK(request.id == 0);
            CHECK(rzv_frame_feed(&frame, bytes + split, sizeof bytes - split,
                                 &used, &request) == RZ_OK &&
                  used == RZV_FRAME_BYTES - split);
        }
        CHECK(request.client == 1 && request.id == 15 &&
              request.fence == UINT64_MAX && request.delta == -8);
        CHECK(rzv_frame_feed(&frame, NULL, 0, &used, &request) == RZ_BUSY &&
              !used && rzv_frame_end(&frame) == RZ_DONE);
    }
    for (unsigned at = 0; at < RZV_FRAME_BYTES; ++at)
    {
        if (at >= 6 && at < 14)
        {
            continue;
        }
        unsigned char old = bytes[at];
        bytes[at] = 255;
        struct rzv_frame frame = {0};
        struct rzj_request request = {.id = 99};
        size_t used = 0;
        CHECK(rzv_frame_feed(&frame, bytes, RZV_FRAME_BYTES, &used, &request) ==
                  RZJ_CORRUPT &&
              request.id == 99 && frame.malformed);
        CHECK(rzv_frame_feed(&frame, NULL, 0, &used, &request) == RZJ_CORRUPT &&
              !used);
        bytes[at] = old;
    }
    for (unsigned n = 0; n < RZV_FRAME_BYTES; ++n)
    {
        struct rzv_frame frame = {0};
        struct rzj_request request = {0};
        size_t used;
        CHECK(rzv_frame_feed(&frame, bytes, n, &used, &request) == RZ_PENDING);
        CHECK(rzv_frame_end(&frame) == (n ? RZJ_CORRUPT : RZ_CLOSED));
    }
    struct rzv_frame frame = {0};
    struct rzj_request request = {.id = 99};
    size_t used = 99;
    CHECK(rzv_frame_feed(&frame, NULL, 1, &used, &request) == RZ_INVALID &&
          used == 99 && request.id == 99);
    for (unsigned i = 0; i < 3; ++i)
    {
        int value = i == 0 ? INT_MIN : i == 1 ? INT_MAX : 0;
        struct rzv_reply reply = {0};
        CHECK(rzv_reply_encode((struct rzv_reply){0, true, value, UINT64_MAX},
                               bytes) == RZ_OK);
        CHECK(rzv_reply_decode(bytes, &reply) == RZ_OK &&
              reply.result == value && reply.duplicate &&
              reply.epoch == UINT64_MAX);
    }
    bytes[0] = 99;
    CHECK(rzv_reply_encode((struct rzv_reply){RZ_FULL, true, 0, 1}, bytes) ==
              RZ_INVALID &&
          bytes[0] == 99);
    CHECK(rzv_reply_encode((struct rzv_reply){19, false, 0, 1}, bytes) ==
              RZ_INVALID &&
          bytes[0] == 99);
}

static void native_partial_and_duplicate(void)
{
    struct rz_fixture f;
    initialize(&f, 2);
    struct rzf_scope *s = scope(&f, 3000);
    int sockets[2][2], cancel[2][2], done[2][2];
    struct rz_worker workers[2];
    pthread_t threads[2];
    for (unsigned i = 0; i < 2; ++i)
    {
        pair(sockets[i], cancel[i]);
        CHECK(!pipe(done[i]));
        send_request(sockets[i][0], (struct rzj_request){0, 1, 1, 3});
        workers[i] =
            (struct rz_worker){&f.service, s,     sockets[i][1], cancel[i][0],
                               done[i][1], i + 1, {0},           0};
        CHECK(!pthread_create(&threads[i], NULL, worker, &workers[i]));
    }
    struct rzv_reply replies[2];
    for (unsigned i = 0; i < 2; ++i)
    {
        replies[i] = response(sockets[i][0]);
    }
    int descriptors[2] = {done[0][0], done[1][0]};
    unsigned completed = 99;
    CHECK(hs_reactor_sum(descriptors, 2, &completed) == 0 &&
          completed ==
              2); /* Retained reactor's exact completion-byte contract. */
    for (unsigned i = 0; i < 2; ++i)
    {
        CHECK(!pthread_join(threads[i], NULL));
        CHECK(workers[i].status == RZ_OK && workers[i].observation.read == 18 &&
              workers[i].observation.written == 18 &&
              workers[i].observation.charged && workers[i].observation.cleaned);
        CHECK(replies[i].status == RZ_OK && replies[i].result == 3);
        close_pair(sockets[i], cancel[i]);
        CHECK(!close(done[i][0]) && !close(done[i][1]));
    }
    CHECK(replies[0].duplicate != replies[1].duplicate &&
          f.journal.receiver.total == 3 && f.outbox.count == 1 &&
          f.service.limiter.tokens == 0 && !f.service.active);
    int p[2], c[2];
    pair(p, c);
    send_request(p[0], (struct rzj_request){0, 1, 1, 3});
    struct rz_worker denied = {&f.service, s, p[1], c[0], -1, 1, {0}, 0};
    worker(&denied);
    struct rzv_reply reply = response(p[0]);
    CHECK(reply.status == RZ_DENIED && !denied.observation.charged &&
          f.journal.receiver.total == 3);
    close_pair(p, c);
    struct rzs_event e;
    CHECK(rzv_notify(&f.service, 0, 0, NULL) == RZ_OK &&
          rzv_notify(&f.service, 1, 0, &e) == RZ_OK &&
          rzv_notify(&f.service, 2, e.sequence, NULL) == RZ_OK);
    rzf_scope_release(&s);
    CHECK(atomic_load(&f.released) == 1);
    destroy(&f);
}

static void failures(void)
{
    for (unsigned mode = 0; mode < 5; ++mode)
    {
        struct rz_fixture f;
        initialize(&f, 4);
        struct rzf_scope *s = scope(&f, mode == 1 ? 0 : 2000);
        int p[2], c[2];
        pair(p, c);
        unsigned char b[RZV_FRAME_BYTES];
        CHECK(rzv_request_encode((struct rzj_request){0, 1, 1, 3}, b) == RZ_OK);
        if (mode == 0)
        {
            CHECK(send(p[0], b, 5, MSG_NOSIGNAL) == 5);
            CHECK(!shutdown(p[0], SHUT_WR));
        }
        else
        {
            CHECK(send(p[0], b, sizeof b, MSG_NOSIGNAL) == (ssize_t)sizeof b);
            if (mode == 2)
            {
                unsigned char one = 1;
                CHECK(write(c[1], &one, 1) == 1);
            }
            if (mode == 3)
            {
                CHECK(!close(p[0]));
                p[0] = -1;
            }
            if (mode == 4)
            {
                rz_cancel(&s->core);
            }
        }
        struct rzv_observation observation = {0};
        int status = rzv_serve(&f.service, s, p[1], c[0], 1, &observation);
        CHECK(status == (mode == 0   ? RZJ_CORRUPT
                         : mode == 1 ? RZ_EXPIRED
                         : mode == 3 ? RZ_CLOSED
                                     : RZ_CANCELLED));
        CHECK(observation.cleaned && !f.service.gate.used[0] &&
              !f.service.active);
        CHECK(f.journal.receiver.total == (mode == 3 ? 3 : 0) &&
              observation.committed == (mode == 3));
        if (p[0] >= 0)
        {
            CHECK(!close(p[0]));
        }
        CHECK(!close(p[1]) && !close(c[0]) && !close(c[1]));
        rzf_scope_release(&s);
        CHECK(atomic_load(&f.released) == 1);
        destroy(&f);
    }
}

static void blocked_reply(void)
{
    struct rz_fixture f;
    initialize(&f, 4);
    struct rzf_scope *s = scope(&f, 3000);
    int p[2], c[2];
    pair(p, c);
    send_request(p[0], (struct rzj_request){0, 1, 1, 3});
    unsigned char filler[4096] = {0};
    size_t filled = 0;
    for (;;)
    {
        ssize_t n = send(p[1], filler, sizeof filler, MSG_NOSIGNAL);
        if (n < 0)
        {
            CHECK(errno == EAGAIN || errno == EWOULDBLOCK);
            break;
        }
        CHECK(n > 0);
        filled += (size_t)n;
    }
    CHECK(filled > 18);
    struct rz_worker w = {&f.service, s, p[1], c[0], -1, 1, {0}, 0};
    pthread_t thread;
    CHECK(!pthread_create(&thread, NULL, worker, &w));
    uint64_t limit = clock_ms() + 2000;
    bool committed = false;
    while (clock_ms() < limit)
    {
        CHECK(!pthread_mutex_lock(&f.service.mutex));
        committed = f.journal.receiver.total == 3;
        unsigned permits = f.service.gate.used[0];
        CHECK(!pthread_mutex_unlock(&f.service.mutex));
        if (committed)
        {
            CHECK(permits == 1);
            break;
        }
        struct timespec pause = {0, 1000000};
        CHECK(!nanosleep(&pause, NULL));
    }
    CHECK(committed && rzv_destroy(&f.service) == RZ_BUSY);
    rzf_scope_release(&s);
    CHECK(!atomic_load(&f.released));
    unsigned char one = 1;
    CHECK(write(c[1], &one, 1) == 1);
    CHECK(!pthread_join(thread, NULL));
    CHECK(w.status == RZ_CANCELLED && w.observation.committed &&
          w.observation.written == 0 && w.observation.cleaned &&
          !f.service.gate.used[0] && atomic_load(&f.released) == 1);
    close_pair(p, c);
    destroy(&f);
}

static void queued_burst(void)
{
    /* All six frames are already queued before this serialized service admits
       one. The two orders vary arrival arbitration, not the ownership contract. */
    for (unsigned reverse = 0; reverse < 2; ++reverse)
    {
        struct rz_fixture f;
        initialize(&f, 8);
        struct rzf_scope *s = scope(&f, 10000);
        int sockets[6][2], cancel[6][2];
        for (unsigned i = 0; i < 6; ++i)
        {
            pair(sockets[i], cancel[i]);
            send_request(sockets[i][0],
                         (struct rzj_request){i % 2, i + 1, 1, 1});
        }
        unsigned admitted_mask = 0;
        for (unsigned order = 0; order < 6; ++order)
        {
            unsigned i = reverse ? 5 - order : order;
            struct rzv_observation observed = {0};
            CHECK(rzv_serve(&f.service, s, sockets[i][1], cancel[i][0], 1,
                            &observed) == RZ_OK);
            struct rzv_reply reply = response(sockets[i][0]);
            CHECK(observed.cleaned && observed.charged);
            CHECK(reply.status == (order < 4 ? RZ_OK : RZ_FULL) &&
                  reply.result == (order < 4 ? (int)order + 1 : 0));
            CHECK(f.journal.receiver.total ==
                      (int)(order < 4 ? order + 1 : 4) &&
                  f.outbox.count == (order < 4 ? order + 1 : 4));
            if (order < 4)
            {
                admitted_mask |= 1u << i;
            }
        }
        /* Reply completion neither forwards nor acknowledges the slow consumer. */
        CHECK(f.outbox.count == 4 && f.outbox.pending.count == 4 &&
              f.service.limiter.tokens == 2 && !f.service.active);
        unsigned delivered_mask = 0;
        for (unsigned order = 0; order < 4; ++order)
        {
            struct rzs_event event;
            CHECK(rzv_notify(&f.service, 0, 0, NULL) == RZ_OK &&
                  rzv_notify(&f.service, 1, 0, &event) == RZ_OK);
            CHECK(event.id >= 1 && event.id <= 6 &&
                  !(delivered_mask & (1u << (event.id - 1))) &&
                  event.result == (int)order + 1);
            delivered_mask |= 1u << (event.id - 1);
            CHECK(f.outbox.count == 4 - order);
            CHECK(rzv_notify(&f.service, 2, event.sequence, NULL) == RZ_OK &&
                  f.outbox.count == 3 - order);
        }
        CHECK(delivered_mask == admitted_mask && f.outbox.ack_records == 4 &&
              f.journal.receiver.total == 4);
        for (unsigned i = 0; i < 6; ++i)
        {
            close_pair(sockets[i], cancel[i]);
        }
        rzf_scope_release(&s);
        CHECK(atomic_load(&f.released) == 1);
        destroy(&f);
    }
}

static void oracle(void)
{
    char hex[73];
    unsigned chunk;
    while (scanf("%u %72s", &chunk, hex) == 2)
    {
        size_t n = strlen(hex) / 2;
        unsigned char bytes[36];
        CHECK(n <= sizeof bytes && chunk >= 1 && chunk <= 18);
        for (size_t i = 0; i < n; ++i)
        {
            unsigned b;
            CHECK(sscanf(hex + 2 * i, "%2x", &b) == 1);
            bytes[i] = (unsigned char)b;
        }
        struct rzv_frame frame = {0};
        struct rzj_request request = {
            .client = 99, .id = 99, .fence = 99, .delta = 99};
        size_t offset = 0;
        int status = RZ_PENDING;
        while (offset < n && status == RZ_PENDING)
        {
            size_t size = n - offset;
            if (size > chunk)
            {
                size = chunk;
            }
            size_t used = 99;
            status =
                rzv_frame_feed(&frame, bytes + offset, size, &used, &request);
            offset += used;
        }
        int ended = rzv_frame_end(&frame);
        printf("%d,%zu,%d,%u,%u,%" PRIu64 ",%d,%u\n", status, offset, ended,
               request.client, request.id, request.fence, request.delta,
               frame.malformed);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "test"))
    {
        parser();
        native_partial_and_duplicate();
        failures();
        blocked_reply();
        queued_burst();
        puts("RZ service I/O passed");
        return 0;
    }
    if (!strcmp(argv[1], "oracle"))
    {
        oracle();
        return 0;
    }
    return 2;
}
