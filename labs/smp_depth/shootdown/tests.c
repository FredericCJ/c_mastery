#include "shootdown.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "ST_CHECK:%u:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

/* Explicit teaching translation caches; no C dangling pointer or real CPU TLB
 * is represented by these ordinary arrays. All frames remain live storage. */
struct sa_map
{
    unsigned valid, writable, frame, global;
};

struct sa_port
{
    struct st_world *world;
    struct sa_map pte[2][4], cache[4][2][4];
    unsigned frame[8], publication, invalidations, notify, early_ack,
        fail_publish, fail_invalidate, fail_notify, inline_handler,
        revoke_write;
};

static int translate(struct sa_port *p, unsigned cpu, unsigned space,
                     unsigned page, int write, unsigned *out)
{
    struct sa_map *cached = &p->cache[cpu][space][page];
    if (!cached->valid)
        *cached = p->pte[space][page];
    if (!cached->valid || (write && !cached->writable))
        return 0;
    if (write)
        p->frame[cached->frame] = *out;
    else
        *out = p->frame[cached->frame];
    return 1;
}

static int publish(void *ctx, struct st_request r)
{
    struct sa_port *p = ctx;
    if (p->fail_publish)
        return 0;
    p->publication++;
    for (unsigned s = 0; s < 2; s++)
        if (s == r.space || r.scope == ST_GLOBAL)
            for (unsigned page = 0; page < r.pages; page++)
            {
                struct sa_map *entry = &p->pte[s][r.first / 4096 - 1 + page];
                if (p->revoke_write)
                    entry->writable = 0;
                else
                    entry->valid = 0;
            }
    return 1;
}

static int invalidate(void *ctx, unsigned cpu, struct st_request r)
{
    struct sa_port *p = ctx;
    if (r.scope != ST_CONTEXT &&
        atomic_load(&p->world->slot[r.space].ack[cpu]) == r.generation)
        p->early_ack++;
    if (p->fail_invalidate)
        return 0;
    p->invalidations++;
    if (r.scope == ST_CONTEXT)
    {
        for (unsigned s = 0; s < 2; s++)
            for (unsigned page = 0; page < 4; page++)
                if (!p->cache[cpu][s][page].global)
                    p->cache[cpu][s][page].valid = 0;
    }
    else
        for (unsigned s = 0; s < 2; s++)
            if (s == r.space || r.scope == ST_GLOBAL)
                for (unsigned page = 0; page < r.pages; page++)
                    p->cache[cpu][s][r.first / 4096 - 1 + page].valid = 0;
    return 1;
}

static int notify(void *ctx, struct st_request r, unsigned targets)
{
    struct sa_port *p = ctx;
    p->notify++;
    if (p->inline_handler)
        for (unsigned cpu = 0; cpu < p->world->cpus; cpu++)
            if (targets & (1u << cpu))
                if (st_handle(p->world, cpu, r) != ST_OK)
                    return 0;
    return !p->fail_notify;
}

static int setup(struct st_world *w, struct sa_port *p, unsigned cpus)
{
    *p = (struct sa_port){0};
    p->world = w;
    for (unsigned s = 0; s < 2; s++)
        for (unsigned i = 0; i < 4; i++)
            p->pte[s][i] = (struct sa_map){1, 1, i, 0};
    for (unsigned i = 0; i < 8; i++)
        p->frame[i] = 100 + i;
    return st_init(w, cpus, 101, 202,
                   (struct st_port){publish, invalidate, notify, p});
}

static int authority(void)
{
    struct st_world w;
    struct sa_port p;
    struct st_request r, old;
    struct st_permit permit = {0}, before = {0};
    struct sl_stats stats = {0};
    unsigned output;
    CHECK(setup(&w, &p, 4) == ST_OK);
    for (unsigned c = 0; c < 3; c++)
        CHECK(st_enter(&w, c, 0, &stats) == ST_OK);
    /* Eight completed requests produce exactly generation9 versus stale ack8. */
    for (unsigned i = 0; i < 8; i++)
    {
        CHECK(st_begin(&w, 0, 0, 4096, 1, ST_PAGE, &r, &stats) == ST_OK);
        for (unsigned c = 0; c < 3; c++)
            CHECK(st_handle(&w, c, r) == ST_OK);
        CHECK(st_finish(&w, r, 0, &permit, &stats) == ST_OK);
    }
    p.pte[0][0] = (struct sa_map){1, 1, 0, 0};
    for (unsigned c = 0; c < 3; c++)
    {
        CHECK(translate(&p, c, 0, 0, 0, &output) && output == 100);
    }
    old = r;
    CHECK(st_begin(&w, 0, 0, 4096, 1, ST_PAGE, &r, &stats) == ST_OK &&
          r.generation == 9 && r.targets == 7);
    CHECK(st_handle(&w, 0, r) == ST_OK);
    CHECK(st_handle(&w, 2, r) == ST_OK);
    CHECK(st_handle(&w, 1, old) == ST_STALE);
    CHECK(st_finish(&w, r, 0, &before, &stats) == ST_INCOMPLETE &&
          before.request.generation == 0);
    CHECK(st_finish(&w, r, 1, &before, &stats) == ST_TIMEOUT &&
          before.request.generation == 0 && atomic_load(&w.slot[0].pending));
    CHECK(st_enter(&w, 3, 0, &stats) == ST_BUSY && w.current[3] == ST_NO_SPACE);
    CHECK(st_leave(&w, 1, &stats) == ST_BUSY);
    CHECK(st_handle(&w, 1, r) == ST_OK);
    CHECK(st_finish(&w, r, 0, &permit, &stats) == ST_OK &&
          !atomic_load(&w.slot[0].pending));
    p.frame[0] = 999;
    CHECK(!translate(&p, 1, 0, 0, 0, &output));
    CHECK(st_enter(&w, 3, 0, &stats) == ST_OK &&
          !translate(&p, 3, 0, 0, 0, &output));
    CHECK(!p.early_ack);
    return 0;
}

static int failure(void)
{
    struct st_world w;
    struct sa_port p;
    struct st_request r = {0}, before = {0};
    struct st_permit permit;
    struct sl_stats stats = {0};
    CHECK(setup(&w, &p, 2) == ST_OK);
    CHECK(st_enter(&w, 0, 0, &stats) == ST_OK);
    CHECK(st_enter(&w, 1, 0, &stats) == ST_OK);
    p.fail_publish = 1;
    CHECK(st_begin(&w, 0, 0, 4096, 1, ST_PAGE, &r, &stats) == ST_PORT &&
          memcmp(&r, &before, sizeof r) == 0 &&
          !atomic_load(&w.slot[0].pending));
    p.fail_publish = 0;
    p.fail_notify = 1;
    CHECK(st_begin(&w, 0, 0, 4096, 1, ST_PAGE, &r, &stats) == ST_SIGNAL &&
          r.generation == 1 && atomic_load(&w.slot[0].pending));
    p.fail_notify = 0;
    p.fail_invalidate = 1;
    CHECK(st_handle(&w, 0, r) == ST_PORT &&
          atomic_load(&w.slot[0].ack[0]) == 0);
    p.fail_invalidate = 0;
    CHECK(st_handle(&w, 0, r) == ST_OK);
    p.inline_handler = 1;
    CHECK(st_retry(&w, r, &stats) == ST_OK);
    CHECK(st_finish(&w, r, 0, &permit, &stats) == ST_OK && !p.early_ack);
    CHECK(st_begin(&w, 0, 0, 4096, 1, ST_PAGE, &r, &stats) == ST_OK);
    CHECK(st_finish(&w, r, 0, &permit, &stats) ==
          ST_OK); /* handler runs while sender owns membership lock */
    CHECK(st_begin(&w, 0, 0, 1, 1, ST_PAGE, &r, &stats) == ST_INVALID);
    CHECK(st_begin(&w, 0, 0, UINTPTR_MAX & ~(uintptr_t)4095, 2, ST_PAGE, &r,
                   &stats) == ST_INVALID);
    CHECK(st_begin(&w, 0, 0, 4096, 0, ST_PAGE, &r, &stats) == ST_INVALID);
    CHECK(st_leave(&w, 0, &stats) == ST_OK);
    CHECK(st_leave(&w, 1, &stats) == ST_OK);
    atomic_store(&w.slot[0].generation, UINT64_MAX - 1);
    p.inline_handler = 0;
    CHECK(st_begin(&w, 0, 0, 4096, 1, ST_PAGE, &r, &stats) == ST_OK &&
          r.generation == UINT64_MAX && r.targets == 0);
    CHECK(st_finish(&w, r, 0, &permit, &stats) == ST_OK);
    CHECK(st_begin(&w, 0, 0, 4096, 1, ST_PAGE, &r, &stats) == ST_FULL);
    CHECK(st_enter(&w, 0, 0, &stats) == ST_OK);
    CHECK(st_leave(&w, 0, &stats) == ST_OK);
    return 0;
}

static int security(void)
{
    struct st_world w;
    struct sa_port p;
    struct st_request r;
    struct st_permit permit;
    struct sl_stats s = {0};
    unsigned value;
    CHECK(setup(&w, &p, 2) == ST_OK);
    CHECK(st_enter(&w, 0, 0, &s) == ST_OK);
    CHECK(st_enter(&w, 1, 0, &s) == ST_OK);
    CHECK(translate(&p, 1, 0, 0, 0, &value) && value == 100);
    CHECK(st_begin(&w, 0, 0, 4096, 1, ST_PAGE, &r, &s) == ST_OK);
    CHECK(st_handle(&w, 0, r) == ST_OK);
    CHECK(st_finish(&w, r, 1, &permit, &s) == ST_TIMEOUT);
    /* Deliberately vulnerable ORIGINAL deterministic target: bypass permit. */
    p.frame[0] = 999;
    CHECK(translate(&p, 1, 0, 0, 0, &value) && value == 999);
    value = 777;
    CHECK(translate(&p, 1, 0, 0, 1, &value) && p.frame[0] == 777);
    CHECK(st_handle(&w, 1, r) == ST_OK);
    CHECK(st_finish(&w, r, 0, &permit, &s) == ST_OK);
    p.frame[0] = 555;
    CHECK(!translate(&p, 1, 0, 0, 0, &value));
    puts(
        "ST_MODEL_WITNESS:early-reuse stale-read=999 stale-write=777 repaired-old-access=refused");
    return 0;
}

static int spaces(void)
{
    struct st_world w;
    struct sa_port p;
    struct st_request a, b;
    struct st_permit permit;
    struct sl_stats s = {0};
    unsigned value = 1;
    CHECK(setup(&w, &p, 4) == ST_OK);
    for (unsigned c = 0; c < 4; c++)
        CHECK(st_enter(&w, c, c / 2, &s) == ST_OK);
    for (unsigned c = 0; c < 4; c++)
        CHECK(translate(&p, c, c / 2, 0, 0, &value));
    p.revoke_write = 1;
    CHECK(st_begin(&w, 0, 0, 4096, 1, ST_PAGE, &a, &s) == ST_OK &&
          a.targets == 3);
    CHECK(st_handle(&w, 0, a) == ST_OK);
    CHECK(st_handle(&w, 1, a) == ST_OK);
    CHECK(st_finish(&w, a, 0, &permit, &s) == ST_OK);
    CHECK(!translate(&p, 1, 0, 0, 1, &value) &&
          translate(&p, 2, 1, 0, 1,
                    &value)); /* second mapping retains frame ownership */
    CHECK(st_begin(&w, 2, 1, 4096, 1, ST_PAGE, &b, &s) == ST_OK &&
          b.targets == 12);
    CHECK(st_handle(&w, 2, b) == ST_OK);
    CHECK(st_handle(&w, 3, b) == ST_OK);
    CHECK(st_finish(&w, b, 0, &permit, &s) == ST_OK);
    CHECK(!translate(&p, 3, 1, 0, 1, &value));
    p.revoke_write = 0;
    for (unsigned sp = 0; sp < 2; sp++)
        p.pte[sp][1] = (struct sa_map){1, 1, 1, 1};
    for (unsigned c = 0; c < 4; c++)
        CHECK(translate(&p, c, c / 2, 1, 0, &value));
    CHECK(st_begin(&w, 0, 0, 8192, 1, ST_GLOBAL, &a, &s) == ST_OK &&
          a.targets == 15);
    CHECK(st_begin(&w, 2, 1, 4096, 1, ST_PAGE, &b, &s) == ST_BUSY);
    for (unsigned c = 0; c < 4; c++)
        CHECK(st_handle(&w, c, a) == ST_OK);
    CHECK(st_finish(&w, a, 0, &permit, &s) == ST_OK);
    p.frame[1] = 999;
    for (unsigned c = 0; c < 4; c++)
        CHECK(!translate(&p, c, c / 2, 1, 0, &value));
    return 0;
}

static int batching(void)
{
    for (unsigned pages = 1; pages <= 4; pages++)
    {
        struct st_world w;
        struct sa_port p;
        struct st_request r;
        struct st_permit permit;
        struct sl_stats s = {0};
        CHECK(setup(&w, &p, 2) == ST_OK);
        CHECK(st_enter(&w, 0, 0, &s) == ST_OK);
        CHECK(st_enter(&w, 1, 0, &s) == ST_OK);
        p.invalidations = 0;
        CHECK(st_begin(&w, 0, 0, 4096, pages, ST_PAGE, &r, &s) == ST_OK);
        CHECK(st_handle(&w, 0, r) == ST_OK);
        CHECK(st_handle(&w, 1, r) == ST_OK);
        CHECK(st_finish(&w, r, 0, &permit, &s) == ST_OK);
        CHECK(p.notify == 1 && p.invalidations == 2 && p.publication == 1);
        printf(
            "ST_BATCH_MODEL pages=%u requests=1 local_operations=2 per_page_invalidations=%u\n",
            pages, 2 * pages);
    }
    return 0;
}

int main(void)
{
    if (authority() || failure() || security() || spaces() || batching())
        return 1;
    puts("ST_PASS:generation-membership-timeout-global-permission-reclaim");
    return 0;
}
