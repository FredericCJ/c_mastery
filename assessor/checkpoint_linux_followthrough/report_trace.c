#include "checkpoint_operation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                          \
    do                                                                    \
    {                                                                     \
        if (!(x))                                                         \
        {                                                                 \
            fprintf(stderr, "P5_TRACE_CHECK line=%d %s\n", __LINE__, #x); \
            exit(1);                                                      \
        }                                                                 \
    } while (0)

struct p5_ctx
{
    struct cp_operation op;
    char trace[9];
    unsigned used, allocations;
    void *live;
};

static void event(struct p5_ctx *c, char e)
{
    CHECK(c->used < sizeof c->trace - 1);
    c->trace[c->used++] = e;
}

static void *allocate(void *p, size_t n)
{
    struct p5_ctx *c = p;
    CHECK(!c->live);
    c->live = malloc(n);
    CHECK(c->live);
    ++c->allocations;
    event(c, 'A');
    return c->live;
}

static int report(void *p, unsigned g, const unsigned char *b, size_t n,
                  int cancelled)
{
    struct p5_ctx *c = p;
    CHECK(c->op.state == CPO_REPORTING && b == c->live && n == 128 &&
          b[127] == 0xa5 && cancelled == (g == 1));
    event(c, 'R');
    unsigned out = 99;
    CHECK(cpo_begin(&c->op, b, n, &out) == CPO_BUSY && out == 99 &&
          cpo_cancel(&c->op, g) == CPO_STALE &&
          cpo_complete(&c->op, g, 1) == CPO_STALE &&
          cpo_destroy(&c->op) == CPO_PENDING);
    return 17;
}

static void release(void *p, void *b)
{
    struct p5_ctx *c = p;
    CHECK(c->op.state == CPO_REPORTING && c->used &&
          c->trace[c->used - 1] == 'R' && b == c->live);
    event(c, 'F');
    free(b);
    c->live = 0;
}

int main(void)
{
    struct p5_ctx c = {0};
    CHECK(cpo_init(&c.op, 2, &c, allocate, release, report) == CPO_OK);
    unsigned char in[128];
    memset(in, 0xa5, sizeof in);
    unsigned g;
    for (unsigned n = 1; n <= 2; n++)
    {
        CHECK(cpo_begin(&c.op, in, sizeof in, &g) == CPO_OK && g == n);
        if (n == 1)
        {
            CHECK(cpo_cancel(&c.op, g) == CPO_PENDING);
        }
        CHECK(cpo_complete(&c.op, g, 1) == CPO_OK && c.op.report_failed == 17 &&
              !c.live && cpo_complete(&c.op, g, 1) == CPO_STALE);
    }
    CHECK(!strcmp(c.trace, "ARFARF") && c.op.state == CPO_DEAD &&
          cpo_destroy(&c.op) == CPO_OK);
    puts(
        "P5_TRACE_PASS allocate_report_free=ARFARF reporter_error=17 reentry_refused generation_retired");
}
