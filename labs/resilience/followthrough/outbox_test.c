#include "outbox.h"
#include "journal_posix.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
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

struct rz_memory
{
    unsigned char cache[8192], durable[8192];
    size_t size, durable_size, chunk;
    unsigned calls, cut, mode;
    bool failed;
};

static void persist(struct rz_memory *m)
{
    for (size_t i = 0; i < m->size; ++i)
        m->durable[i] = m->cache[i];
    m->durable_size = m->size;
}

static void reset(struct rz_memory *m)
{
    m->size = m->durable_size;
    for (size_t i = 0; i < m->size; ++i)
        m->cache[i] = m->durable[i];
    m->failed = false;
    m->cut = m->calls = 0;
}

static int event(struct rz_memory *m)
{
    ++m->calls;
    if (m->cut != m->calls)
        return RZ_OK;
    if (m->mode)
        persist(m);
    reset(m);
    m->failed = true;
    return RZJ_UNCERTAIN;
}

static int size(void *p, size_t *out)
{
    *out = ((struct rz_memory *)p)->size;
    return RZ_OK;
}

static int read_bytes(void *p, size_t at, unsigned char *out, size_t n,
                      size_t *done)
{
    struct rz_memory *m = p;
    if (at > m->size)
        return RZ_INVALID;
    if (n > m->size - at)
        n = m->size - at;
    if (m->chunk && n > m->chunk)
        n = m->chunk;
    for (size_t i = 0; i < n; ++i)
        out[i] = m->cache[at + i];
    *done = n;
    return RZ_OK;
}

static int write_bytes(void *p, size_t at, const unsigned char *in, size_t n,
                       size_t *done)
{
    struct rz_memory *m = p;
    if (m->failed || at > sizeof m->cache || n > sizeof m->cache - at)
        return RZJ_UNCERTAIN;
    if (m->chunk && n > m->chunk)
        n = m->chunk;
    for (size_t i = 0; i < n; ++i)
        m->cache[at + i] = in[i];
    if (at + n > m->size)
        m->size = at + n;
    *done = n;
    return event(m);
}

static int truncate_bytes(void *p, size_t n)
{
    struct rz_memory *m = p;
    if (n > m->size)
        return RZ_INVALID;
    m->size = n;
    return event(m);
}

static int flush(void *p)
{
    struct rz_memory *m = p;
    if (m->failed)
        return RZJ_UNCERTAIN;
    int status = event(m);
    if (status == RZ_OK)
        persist(m);
    return status;
}

static struct rzj_io io(struct rz_memory *m)
{
    return (struct rzj_io){m,           size,           read_bytes,
                           write_bytes, truncate_bytes, flush};
}

struct rz_fixture
{
    struct rz_memory log, ack;
    struct rzj_journal journal;
    struct rzs_outbox service;
    struct msg_hub hub;
};

static void initialize(struct rz_fixture *f, unsigned capacity, unsigned window)
{
    *f = (struct rz_fixture){.log = {.chunk = 7}, .ack = {.chunk = 3}};
    CHECK(msg_hub_init(&f->hub, capacity, 1) == MSG_OK);
    CHECK(rzj_open(io(&f->log), window, &f->journal) == RZ_OK);
    uint64_t epoch;
    CHECK(rzj_issue(&f->journal, 0, &epoch) == RZ_OK && epoch == 1);
    CHECK(rzs_open(&f->journal, io(&f->ack), &f->hub, &f->service) == RZ_OK);
}

static void restart(struct rz_fixture *f, unsigned capacity, unsigned window)
{
    reset(&f->log);
    reset(&f->ack);
    msg_hub_destroy(&f->hub);
    CHECK(msg_hub_init(&f->hub, capacity, 1) == MSG_OK);
    CHECK(rzj_open(io(&f->log), window, &f->journal) == RZ_OK);
    CHECK(rzs_open(&f->journal, io(&f->ack), &f->hub, &f->service) == RZ_OK);
}

static int apply(struct rz_fixture *f, uint64_t now, unsigned id, int delta)
{
    int value = 99;
    bool duplicate = true;
    return rzs_apply(&f->service, now,
                     (struct rzj_request){0, id, f->service.epoch, delta},
                     &value, &duplicate);
}

static struct rzs_event deliver(struct rz_fixture *f)
{
    struct rzs_event event = {0};
    CHECK(rzs_forward(&f->service) == RZ_OK);
    CHECK(rzs_deliver(&f->service, &event) == RZ_OK);
    return event;
}

static void nominal(void)
{
    struct rz_fixture f;
    initialize(&f, 1, 4);
    int result = 99;
    bool duplicate = true;
    CHECK(rzs_apply(&f.service, 0, (struct rzj_request){0, 1, 1, 3}, &result,
                    &duplicate) == RZ_OK &&
          result == 3 && !duplicate);
    CHECK(f.service.count == 1 && f.service.pending.count == 1 &&
          f.journal.receiver.total == 3);
    CHECK(rzs_apply(&f.service, 1, (struct rzj_request){0, 1, 1, 3}, &result,
                    &duplicate) == RZ_OK &&
          result == 3 && duplicate && f.service.count == 1);
    CHECK(rzs_apply(&f.service, 1, (struct rzj_request){0, 1, 1, 4}, &result,
                    &duplicate) == RZ_CONFLICT);
    struct rzs_event e = deliver(&f);
    CHECK(e.sequence == 2 && e.epoch == 1 && e.result == 3 && e.state == 3);
    uint64_t epoch = 99;
    CHECK(rzs_rotate(&f.service, 1, &epoch) == RZ_BUSY && epoch == 99);
    CHECK(rzs_ack(&f.service, 3) == RZ_STALE && f.service.count == 1);
    restart(&f, 1, 4);
    CHECK(f.service.count == 1 && f.service.events[0].state == 1 &&
          f.journal.receiver.total == 3);
    CHECK(rzs_ack(&f.service, e.sequence) == RZ_STALE);
    e = deliver(&f);
    CHECK(e.sequence == 2);
    CHECK(rzs_ack(&f.service, e.sequence) == RZ_OK && !f.service.count);
    CHECK(rzs_ack(&f.service, e.sequence) == RZ_STALE);
    restart(&f, 1, 4);
    CHECK(!f.service.count && f.service.acknowledged == 2);
    CHECK(apply(&f, 4, 1, 3) == RZ_STALE && f.journal.receiver.total == 3);
    CHECK(rzs_rotate(&f.service, 4, &epoch) == RZ_OK && epoch == 2);
    CHECK(rzs_apply(&f.service, 4, (struct rzj_request){0, 2, 1, 3}, &result,
                    &duplicate) == RZ_STALE);
    CHECK(apply(&f, 4, 1, 3) == RZ_OK && f.journal.receiver.total == 6);
    restart(&f, 1, 4);
    CHECK(f.service.epoch == 2 && f.service.seen[0] == 2 &&
          f.service.count == 1 && f.journal.receiver.total == 6);
    msg_hub_destroy(&f.hub);

    initialize(&f, 1, 2);
    for (unsigned i = 1; i <= 4; ++i)
        CHECK(apply(&f, i * 2, i, 1) == RZ_OK);
    unsigned records = f.journal.records;
    CHECK(apply(&f, 10, 5, 1) == RZ_FULL && f.journal.receiver.total == 4 &&
          f.journal.records == records);
    CHECK(rzs_forward(&f.service) == RZ_OK &&
          rzs_forward(&f.service) == RZ_FULL && f.service.pending.count == 3 &&
          f.service.count == 4);
    CHECK(rzs_deliver(&f.service, &e) == RZ_OK && e.sequence == 2);
    CHECK(rzs_ack(&f.service, 2) == RZ_OK);
    CHECK(apply(&f, 10, 5, 1) == RZ_OK && f.journal.receiver.total == 5 &&
          f.service.count == 4);
    CHECK(msg_close(&f.hub, 0) == MSG_OK &&
          rzs_forward(&f.service) == RZ_CLOSED && f.service.count == 4);
    restart(&f, 1, 2);
    for (unsigned i = 0; i < 4; ++i)
    {
        e = deliver(&f);
        CHECK(rzs_ack(&f.service, e.sequence) == RZ_OK);
    }
    CHECK(!f.service.count);
    msg_hub_destroy(&f.hub);
}

static void extended_identity(void)
{
    struct rz_fixture f;
    initialize(&f, 1, 2);
    uint64_t now = 0;
    for (unsigned round = 0; round < 3; ++round)
    {
        for (unsigned id = 1; id <= 15; ++id)
        {
            CHECK(apply(&f, now, id, 1) == RZ_OK);
            struct rzs_event e = deliver(&f);
            CHECK(e.epoch == round + 1 && e.id == id);
            CHECK(rzs_ack(&f.service, e.sequence) == RZ_OK);
            now += 2;
        }
        CHECK(apply(&f, now, 1, 1) == RZ_STALE &&
              f.journal.receiver.total == (int)((round + 1) * 15));
        restart(&f, 1, 2);
        CHECK(apply(&f, now, 1, 1) == RZ_STALE);
        uint64_t epoch;
        CHECK(rzs_rotate(&f.service, now, &epoch) == RZ_OK &&
              epoch == round + 2);
    }
    CHECK(f.journal.receiver.total == 45);
    msg_hub_destroy(&f.hub);
}

static void cuts(void)
{
    for (unsigned operation = 0; operation < 3; ++operation)
        for (unsigned mode = 0; mode < 2; ++mode)
            for (unsigned cut = 1; cut <= 69; ++cut)
            {
                struct rz_fixture f;
                initialize(&f, 1, 4);
                struct rz_memory *device = operation == 1 ? &f.ack : &f.log;
                struct rzs_event e = {0};
                if (operation == 1)
                {
                    CHECK(apply(&f, 0, 1, 3) == RZ_OK);
                    e = deliver(&f);
                }
                device->chunk = 1;
                device->calls = 0;
                device->cut = cut;
                device->mode = mode;
                uint64_t epoch = 99;
                int status = operation == 0 ? apply(&f, 1, 1, 3)
                             : operation == 1
                                 ? rzs_ack(&f.service, e.sequence)
                                 : rzs_rotate(&f.service, 1, &epoch);
                CHECK(status == RZ_OK || status == RZJ_UNCERTAIN);
                if (status == RZJ_UNCERTAIN)
                    CHECK(operation == 1 ? f.service.count == 1
                                         : f.journal.receiver.total == 0);
                restart(&f, 1, 4);
                if (operation == 0)
                {
                    CHECK(f.journal.receiver.total == 0 ||
                          f.journal.receiver.total == 3);
                    CHECK(f.service.count ==
                          (unsigned)(f.journal.receiver.total == 3));
                    if (status == RZ_OK)
                        CHECK(f.service.count == 1);
                }
                if (operation == 1)
                {
                    CHECK(f.journal.receiver.total == 3 &&
                          f.service.count <= 1);
                    if (status == RZ_OK)
                        CHECK(f.service.count == 0);
                    if (f.service.count)
                    {
                        e = deliver(&f);
                        CHECK(rzs_ack(&f.service, e.sequence) == RZ_OK);
                    }
                }
                if (operation == 2)
                {
                    CHECK(f.service.epoch == 1 || f.service.epoch == 2);
                    if (status == RZ_OK)
                        CHECK(f.service.epoch == 2);
                }
                msg_hub_destroy(&f.hub);
            }
}

static void invalid_recovery(void)
{
    struct rz_fixture f;
    initialize(&f, 1, 4);
    CHECK(apply(&f, 0, 1, 3) == RZ_OK);
    struct rzs_event e = deliver(&f);
    CHECK(rzs_ack(&f.service, e.sequence) == RZ_OK);
    struct rzs_outbox before = f.service;
    f.ack.cache[8] ^= 1;
    CHECK(rzs_open(&f.journal, io(&f.ack), &f.hub, &f.service) == RZJ_CORRUPT &&
          f.service.acknowledged == before.acknowledged);
    f.ack.cache[8] ^= 1;
    for (unsigned n = 1; n < 32; ++n)
    {
        struct rz_memory original = f.ack;
        for (unsigned i = 0; i < n; ++i)
            f.ack.cache[f.ack.size++] = (unsigned char)i;
        persist(&f.ack);
        CHECK(rzs_open(&f.journal, io(&f.ack), &f.hub, &f.service) == RZ_OK &&
              f.ack.size == 32 && f.ack.durable_size == 32);
        f.ack = original;
    }
    CHECK(msg_send(&f.hub, 0, 999, false) == MSG_OK);
    CHECK(rzs_open(&f.journal, io(&f.ack), &f.hub, &f.service) == RZ_BUSY);
    msg_hub_destroy(&f.hub);
}

static void native(void)
{
    char lp[] = "/tmp/cmastery-rzs-log-XXXXXX",
         ap[] = "/tmp/cmastery-rzs-ack-XXXXXX";
    int lf = mkstemp(lp), af = mkstemp(ap);
    CHECK(lf >= 0 && af >= 0);
    CHECK(!unlink(lp) && !unlink(ap));
    struct rzj_posix l, a;
    struct rzj_io li, ai;
    CHECK(rzj_posix(lf, 3, &l, &li) == RZ_OK &&
          rzj_posix(af, 2, &a, &ai) == RZ_OK);
    struct rzj_journal j;
    CHECK(rzj_open(li, 4, &j) == RZ_OK);
    uint64_t epoch;
    CHECK(rzj_issue(&j, 0, &epoch) == RZ_OK);
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        struct msg_hub hub;
        CHECK(msg_hub_init(&hub, 1, 1) == MSG_OK);
        struct rzs_outbox s;
        CHECK(rzs_open(&j, ai, &hub, &s) == RZ_OK);
        int result;
        bool duplicate;
        CHECK(rzs_apply(&s, 0, (struct rzj_request){0, 1, 1, 3}, &result,
                        &duplicate) == RZ_OK);
        CHECK(rzs_forward(&s) == RZ_OK);
        struct rzs_event e;
        CHECK(rzs_deliver(&s, &e) == RZ_OK);
        _exit(0);
    }
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    CHECK(rzj_open(li, 4, &j) == RZ_OK && j.receiver.total == 3);
    struct msg_hub hub;
    CHECK(msg_hub_init(&hub, 1, 1) == MSG_OK);
    struct rzs_outbox s;
    CHECK(rzs_open(&j, ai, &hub, &s) == RZ_OK && s.count == 1);
    int result;
    bool duplicate;
    CHECK(rzs_apply(&s, 1, (struct rzj_request){0, 1, 1, 3}, &result,
                    &duplicate) == RZ_OK &&
          duplicate && result == 3 && s.count == 1);
    CHECK(rzs_forward(&s) == RZ_OK);
    struct rzs_event e;
    CHECK(rzs_deliver(&s, &e) == RZ_OK && e.sequence == 2);
    CHECK(rzs_ack(&s, e.sequence) == RZ_OK);
    msg_hub_destroy(&hub);
    CHECK(!close(lf) && !close(af));
}

static void oracle(void)
{
    unsigned capacity;
    char word[64];
    while (scanf("%u %63s", &capacity, word) == 2)
    {
        struct rz_fixture f;
        initialize(&f, capacity, 2);
        uint64_t now = 0;
        for (size_t i = 0; word[i]; ++i)
        {
            int status = RZ_OK;
            if (word[i] == 'a' || word[i] == 'b')
                status = apply(&f, now, word[i] == 'a' ? 1 : 2,
                               word[i] == 'a' ? 1 : 2);
            else if (word[i] == 't')
                ++now;
            else if (word[i] == 'f')
                status = rzs_forward(&f.service);
            else if (word[i] == 'd')
            {
                struct rzs_event e;
                status = rzs_deliver(&f.service, &e);
            }
            else if (word[i] == 'k')
                status =
                    rzs_ack(&f.service,
                            f.service.count ? f.service.events[0].sequence : 0);
            else if (word[i] == 'r')
                restart(&f, capacity, 2);
            else if (word[i] == 'e')
            {
                uint64_t epoch;
                status = rzs_rotate(&f.service, now, &epoch);
            }
            else
                CHECK(false);
            printf("%d,%d,%" PRIu64 ",%u,%zu,%u,%u,%u:", status,
                   f.journal.receiver.total, f.service.epoch, f.service.count,
                   f.service.pending.count, f.service.acknowledged,
                   f.journal.records, f.service.seen[0]);
            for (unsigned k = 0; k < f.service.count; ++k)
                printf("%u.%u/", f.service.events[k].sequence,
                       f.service.events[k].state);
            putchar(';');
        }
        putchar('\n');
        msg_hub_destroy(&f.hub);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "test"))
    {
        nominal();
        extended_identity();
        cuts();
        invalid_recovery();
        native();
        puts("RZ outbox passed");
        return 0;
    }
    if (!strcmp(argv[1], "oracle"))
    {
        oracle();
        return 0;
    }
    return 2;
}
