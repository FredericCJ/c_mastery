#include "replacement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "LP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

/* Ledger of the callbacks. live[id - 1] is the block acquire() handed out for
   resource id and not yet released. */
struct lp_track
{
    void *live[2];
    unsigned fail; /* id whose acquisition is refused; 0 for none */
    unsigned calls;
    unsigned releases;
    unsigned order; /* released ids as decimal digits: 21 is 2 then 1 */
    unsigned fault; /* releases of an unknown id or block */
    struct le_owner *observed; /* owner inspected at each release */
    unsigned empty; /* releases that saw the observed owner already empty */
};
static struct lp_track t;

static void cleanup(void)
{
    free(t.live[0]);
    free(t.live[1]);
    t = (struct lp_track){0};
}

static void *acquire(void *c, unsigned id, size_t n)
{
    struct lp_track *s = c;
    s->calls++;
    if (s->fail == id)
    {
        return NULL;
    }
    void *p = malloc(n);
    if (!p)
    {
        abort();
    }
    s->live[id - 1] = p;
    return p;
}

static void release(void *c, unsigned id, void *p, size_t n)
{
    struct lp_track *s = c;
    (void)n;
    if (id < 1 || id > 2 || s->live[id - 1] != p)
    {
        s->fault++;
        return;
    }
    s->releases++;
    s->order = s->order * 10 + id;
    if (s->observed)
    {
        s->empty += s->observed->data == NULL && s->observed->size == 0 &&
                    s->observed->id == 0;
    }
    s->live[id - 1] = NULL;
    free(p);
}

static struct le_ops ops(void)
{
    return (struct le_ops){&t, acquire, release};
}

static int failure(void)
{
    struct le_owner out = {0};
    t.fail = 2;
    CHECK(le_run(ops(), 0, &out) == LE_NOMEM && t.calls == 2 &&
          t.releases == 1 && t.order == 1 && !out.data);
    puts("LP failure validated");
    cleanup();
    return 0;
}

static int order(void)
{
    struct le_owner out = {0};
    CHECK(le_run(ops(), 0, &out) == LE_OK && t.order == 21 && t.releases == 2 &&
          !out.data);
    puts("LP order validated");
    cleanup();
    return 0;
}

static int transfer(void)
{
    for (unsigned id = 1; id <= 2; id++)
    {
        struct le_owner out = {0};
        /* Observe the independent release ledger before using the
           transferred pointer. */
        CHECK(le_run(ops(), id, &out) == LE_OK);
        CHECK(t.releases == 1 && t.order == 3 - id);
        CHECK(out.data && out.id == id && out.data[0] == (id == 1 ? 1 : 17));
        t.observed = &out;
        le_drop(&out);
        CHECK(!out.data && !out.size && !out.id && t.empty == 1 &&
              t.releases == 2);
        le_drop(&out);
        CHECK(t.releases == 2 && !t.fault);
        cleanup();
    }
    puts("LP transfer validated");
    return 0;
}

/* Read "fail transfer" records and print the ledger before and after the
   returned owner is dropped. */
static int oracle(void)
{
    unsigned fail;
    unsigned tr;
    while (scanf("%u %u", &fail, &tr) == 2)
    {
        struct le_owner out = {0};
        t.fail = fail;
        enum le_status s = le_run(ops(), tr, &out);
        unsigned id = out.id;
        printf("%d %u %u %u %u", s, t.calls, t.releases, t.order, id);
        le_drop(&out);
        printf(" %u %u %u\n", t.releases, t.order, t.fault);
        cleanup();
    }
    return 0;
}

static int metrics(void)
{
    printf(
        "{\"owner_bytes\":%zu,\"resource_bytes\":24,\"maximum_acquisitions\":2}\n",
        sizeof(struct le_owner));
    return 0;
}

struct lp_case
{
    const char *name;
    int (*run)(void);
};

static const struct lp_case cases[] = {{"failure", failure},
                                       {"order", order},
                                       {"transfer", transfer},
                                       {"oracle", oracle},
                                       {"metrics", metrics}};

/* Exit status: 0 pass, 1 failed check, 2 unknown case name. */
int main(int argc, char **argv)
{
    CHECK(argc == 2);
    atexit(cleanup);
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            return cases[i].run();
        }
    }
    return 2;
}
