#include "async.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "SA_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

struct store_fake
{
    void *borrow;
    int64_t submit, wait;
    unsigned submits, waits;
};

static int64_t submit(void *c, uint32_t page, void *b)
{
    struct store_fake *f = c;
    f->submits++;
    if (page != 7)
        return -1;
    if (f->submit > 0)
        f->borrow = b;
    return f->submit;
}

static int64_t wait_for(void *c, uint64_t ticket)
{
    struct store_fake *f = c;
    f->waits++;
    if (ticket != (uint64_t)f->submit)
        return -1;
    if (!f->wait)
    {
        for (unsigned i = 0; i < 4096; i++)
            ((uint8_t *)f->borrow)[i] = (uint8_t)(i * 13 + 7);
        f->borrow = 0;
    }
    return f->wait;
}

static int retained(void)
{
    struct store_fake f = {0, 1, 0, 0, 0};
    struct sa_port p = {&f, submit, wait_for};
    struct sa_read s = {0};
    uint8_t b[4096] = {0};
    void *out = (void *)&f;
    CHECK(sa_start(&s, p, 7, b) == SA_OK && f.borrow == b);
    CHECK(sa_start(&s, p, 7, b) == SA_BUSY && f.submits == 1);
    CHECK(sa_abandon(&s) == SA_BUSY);
    CHECK(sa_release(&s, &out) == SA_BUSY && out == &f && f.borrow == b);
    CHECK(sa_finish(&s) == SA_DRAINED && f.waits == 1 && !f.borrow);
    for (unsigned i = 0; i < 4096; i++)
        CHECK(b[i] == (uint8_t)(i * 13 + 7));
    CHECK(sa_finish(&s) == SA_DRAINED && f.waits == 1);
    CHECK(sa_release(&s, &out) == SA_OK && out == b && s.state == SA_EMPTY);
    CHECK(sa_start(&s, p, 7, b) == SA_OK);
    CHECK(sa_finish(&s) == SA_READY);
    CHECK(sa_release(&s, &out) == SA_OK);
    puts("SA retained validated "
         "abandonment-retains-borrow-until-positive-completion");
    return 0;
}

static int unknown(void)
{
    struct store_fake f = {0, 2, -1, 0, 0};
    struct sa_read s = {0};
    struct sa_port p = {&f, submit, wait_for};
    uint8_t b[4096] = {0};
    void *out = &f;
    CHECK(sa_start(&s, p, 7, b) == SA_OK);
    CHECK(sa_finish(&s) == SA_UNKNOWN && f.borrow == b);
    CHECK(sa_release(&s, &out) == SA_UNKNOWN && out == &f && f.borrow == b);
    CHECK(sa_abandon(&s) == SA_UNKNOWN);
    f.wait = 0;
    CHECK(sa_finish(&s) == SA_UNKNOWN && f.waits == 1);
    CHECK(sa_start(&s, p, 7, b) == SA_BUSY);
    puts(
        "SA unknown validated terminal-error-not-quiescence-no-automatic-reuse");
    return 0;
}

static int admission(void)
{
    struct store_fake f = {0, -2, 0, 0, 0};
    struct sa_read s = {0};
    struct sa_port p = {&f, submit, wait_for};
    uint8_t b[4096];
    CHECK(sa_start(&s, p, 7, b) == SA_IO && s.state == SA_EMPTY && !s.buffer &&
          !s.ticket && !f.borrow);
    CHECK(sa_start(&s, p, 0, b) == SA_IO && s.state == SA_EMPTY && !s.buffer &&
          !s.ticket);
    CHECK(sa_start(&s, p, 7, 0) == SA_INVALID);
    CHECK(sa_finish(&s) == SA_INVALID);
    CHECK(sa_release(&s, 0) == SA_INVALID);
    puts("SA admission validated "
         "no-request-on-refusal-live-control-reuse-refused");
    return 0;
}

static int oracle(void)
{
    char path[8];
    int accepted, completed;
    while (scanf("%d %d %7s", &accepted, &completed, path) == 3)
    {
        struct store_fake f = {0, accepted ? 1 : -1, completed ? 0 : -1, 0, 0};
        struct sa_read s = {0};
        struct sa_port p = {&f, submit, wait_for};
        uint8_t b[4096] = {0};
        void *out = &f;
        for (unsigned i = 0; path[i]; i++)
        {
            int r = path[i] == 'S'   ? sa_start(&s, p, 7, b)
                    : path[i] == 'A' ? sa_abandon(&s)
                    : path[i] == 'F' ? sa_finish(&s)
                                     : sa_release(&s, &out);
            printf("%d,%u,%u,%llu,%u,%u,%u,%u;", r, s.state, s.abandoned,
                   (unsigned long long)s.ticket, f.submits, f.waits,
                   f.borrow != 0, out == b);
        }
        putchar('\n');
    }
    return 0;
}

int main(int n, char **v)
{
    if (n != 2)
        return 2;
    return !strcmp(v[1], "oracle")      ? oracle()
           : !strcmp(v[1], "retained")  ? retained()
           : !strcmp(v[1], "unknown")   ? unknown()
           : !strcmp(v[1], "admission") ? admission()
                                        : 2;
}
